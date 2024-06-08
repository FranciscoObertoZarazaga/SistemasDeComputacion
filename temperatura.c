#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/w1.h>
#include <linux/file.h>
#include <linux/version.h>

#define DRIVER_NAME "ds18b20_driver"
#define DRIVER_CLASS "DS18B20Class"
#define W1_BUS_MASTER_PATH "/sys/bus/w1/devices/"
#define SENSOR_ID "28-3c01d6070c02"
#define W1_SLAVE_PATH W1_BUS_MASTER_PATH SENSOR_ID "/w1_slave"
#define OUTPUT_FILE_PATH "/tmp/temperature_data.txt"

static dev_t my_device_nr;
static struct class *my_class;
static struct cdev my_device;
static struct task_struct *task;
static int device_open = 0;

static int read_temperature(char *buffer, size_t size);
static int parse_temperature(const char *buffer);
static int write_temperature_to_file(const char *data);
static ssize_t driver_read(struct file *file, char __user *user_buf, size_t count, loff_t *pos);

static int read_temperature(char *buffer, size_t size) {
    struct file *f;
    loff_t pos = 0;
    int ret;

    printk(KERN_INFO "Attempting to open %s\n", W1_SLAVE_PATH);  // Mensaje de depuración

    f = filp_open(W1_SLAVE_PATH, O_RDONLY, 0);
    if (IS_ERR(f)) {
        printk(KERN_ERR "Error opening w1_slave file: %ld\n", PTR_ERR(f));
        return PTR_ERR(f);
    }

    ret = kernel_read(f, buffer, size, &pos);
    filp_close(f, NULL);

    if (ret < 0) {
        printk(KERN_ERR "Error reading w1_slave file\n");
    }

    return ret;
}

static int parse_temperature(const char *buffer) {
    const char *temp_str;
    int temp;

    temp_str = strstr(buffer, "t=");
    if (temp_str) {
        temp = simple_strtol(temp_str + 2, NULL, 10);
        return temp / 1000;  // Convert to Celsius
    }

    return -1;  // Error parsing temperature
}

static int write_temperature_to_file(const char *data) {
    struct file *f;
    loff_t pos = 0;
    ssize_t ret;

    f = filp_open(OUTPUT_FILE_PATH, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (IS_ERR(f)) {
        printk(KERN_ERR "Error opening output file\n");
        return PTR_ERR(f);
    }

    ret = kernel_write(f, data, strlen(data), &pos);
    filp_close(f, NULL);

    if (ret < 0) {
        printk(KERN_ERR "Error writing to output file\n");
        return ret;
    }

    return 0;
}

static int sensor_thread(void *data) {
    char temp_data[128];
    int temperature;

    while (!kthread_should_stop()) {
        if (read_temperature(temp_data, sizeof(temp_data)) >= 0) {
            temperature = parse_temperature(temp_data);
            if (temperature >= 0) {
                char temp_str[32];
                snprintf(temp_str, sizeof(temp_str), "Temp: %d\n", temperature);
                printk(KERN_INFO "Writing temperature to file: %s\n", temp_str);  // Mensaje de depuración
                write_temperature_to_file(temp_str);
            } else {
                printk(KERN_ERR "Failed to parse temperature\n");
            }
        } else {
            printk(KERN_ERR "Failed to read temperature\n");
        }

        msleep(1000);  // Send data every 1 second
    }

    return 0;
}

static int driver_open(struct inode *inode, struct file *file) {
    if (device_open) {
        return -EBUSY;
    }
    device_open++;
    try_module_get(THIS_MODULE);

    // Start the sensor thread
    task = kthread_run(sensor_thread, NULL, "sensor_thread");
    if (IS_ERR(task)) {
        printk("Error creating sensor thread.\n");
        device_open--;
        module_put(THIS_MODULE);
        return PTR_ERR(task);
    }

    return 0;
}

static int driver_release(struct inode *inode, struct file *file) {
    device_open--;
    module_put(THIS_MODULE);

    // Stop the sensor thread
    if (task) {
        kthread_stop(task);
        task = NULL;
    }

    return 0;
}

static ssize_t driver_read(struct file *file, char __user *user_buf, size_t count, loff_t *pos) {
    char buffer[128];
    int temperature;
    char temp_str[32];
    int len;

    if (*pos > 0)
        return 0;

    if (read_temperature(buffer, sizeof(buffer)) >= 0) {
        temperature = parse_temperature(buffer);
        if (temperature >= 0) {
            snprintf(temp_str, sizeof(temp_str), "Temp: %d\n", temperature);
            len = strlen(temp_str);

            if (copy_to_user(user_buf, temp_str, len)) {
                return -EFAULT;
            }

            *pos += len;
            return len;
        }
    }

    return -EINVAL;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = driver_open,
    .release = driver_release,
    .read = driver_read
};

static int __init ModuleInit(void) {
    printk("Initializing DS18B20 driver...\n");

    // Allocate device number
    if (alloc_chrdev_region(&my_device_nr, 0, 1, DRIVER_NAME) < 0) {
        printk("Device number allocation failed!\n");
        return -1;
    }

    // Create device class
    if ((my_class = class_create(DRIVER_CLASS)) == NULL) {
        printk("Device class creation failed!\n");
        goto ClassError;
    }

    // Create device file
    if (device_create(my_class, NULL, my_device_nr, NULL, DRIVER_NAME) == NULL) {
        printk("Device file creation failed!\n");
        goto FileError;
    }

    // Initialize and add cdev
    cdev_init(&my_device, &fops);
    if (cdev_add(&my_device, my_device_nr, 1) == -1) {
        printk("cdev add failed!\n");
        goto AddError;
    }

    printk("DS18B20 driver initialized.\n");
    return 0;

AddError:
    device_destroy(my_class, my_device_nr);
FileError:
    class_destroy(my_class);
ClassError:
    unregister_chrdev_region(my_device_nr, 1);
    return -1;
}

static void __exit ModuleExit(void) {
    if (device_open) {
        kthread_stop(task);
    }
    cdev_del(&my_device);
    device_destroy(my_class, my_device_nr);
    class_destroy(my_class);
    unregister_chrdev_region(my_device_nr, 1);
    printk("DS18B20 driver removed.\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GRUPO_LIPCEN_OBERTO_FERNANDEZ");
MODULE_DESCRIPTION("Driver de kernel para DS18B20");
