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
#include <net/sock.h>
#include <net/tcp.h>
#include <linux/inet.h>  // para in4_pton
#include <linux/file.h>  // para struct file y funciones de archivos

#define DRIVER_NAME "ds18b20_driver"
#define DRIVER_CLASS "DS18B20Class"
#define W1_BUS_MASTER_PATH "/sys/bus/w1/devices/"

#define SERVER_PORT 8001
#define SERVER_ADDR "127.0.0.1"

static dev_t my_device_nr;
static struct class *my_class;
static struct cdev my_device;
static struct socket *sock;
static struct task_struct *task;
static int device_open = 0;

static int setup_socket(void);
static void close_socket(void);
static int send_data_to_server(const char *data);

static int setup_socket(void) {
    struct sockaddr_in server;
    int ret;

    ret = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
    if (ret < 0) {
        printk(KERN_ERR "Error creating socket: %d\n", ret);
        return ret;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    if (in4_pton(SERVER_ADDR, -1, (u8 *)&server.sin_addr.s_addr, -1, NULL) == 0) {
        printk(KERN_ERR "Invalid server address\n");
        sock_release(sock);
        return -EINVAL;
    }
    server.sin_port = htons(SERVER_PORT);

    ret = sock->ops->connect(sock, (struct sockaddr *)&server, sizeof(server), 0);
    if (ret < 0) {
        printk(KERN_ERR "Error connecting to server: %d\n", ret);
        sock_release(sock);
        return ret;
    }

    return 0;
}

static void close_socket(void) {
    if (sock) {
        sock_release(sock);
    }
}

static int send_data_to_server(const char *data) {
    struct msghdr msg;
    struct kvec vec;
    int len, ret;

    len = strlen(data);
    memset(&msg, 0, sizeof(msg));
    vec.iov_base = (void *)data;
    vec.iov_len = len;

    ret = kernel_sendmsg(sock, &msg, &vec, 1, len);
    if (ret < 0) {
        printk(KERN_ERR "Error sending data to server: %d\n", ret);
        return ret;
    }

    return 0;
}

static int read_temperature(char *buffer, size_t size) {
    struct file *f;
    loff_t pos = 0;
    int ret;

    f = filp_open(W1_BUS_MASTER_PATH "28-xxxxxxxxxxxx/w1_slave", O_RDONLY, 0);
    if (IS_ERR(f)) {
        printk(KERN_ERR "Error opening w1_slave file\n");
        return PTR_ERR(f);
    }

    ret = kernel_read(f, buffer, size, &pos);

    filp_close(f, NULL);

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

static int sensor_thread(void *data) {
    char temp_data[128];
    int temperature;

    while (!kthread_should_stop()) {
        if (read_temperature(temp_data, sizeof(temp_data)) >= 0) {
            temperature = parse_temperature(temp_data);
            if (temperature >= 0) {
                char temp_str[16];
                snprintf(temp_str, sizeof(temp_str), "Temp: %d\n", temperature);
                send_data_to_server(temp_str);
            }
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

    // Setup socket and start the sensor thread
    if (setup_socket() < 0) {
        return -EIO;
    }

    task = kthread_run(sensor_thread, NULL, "sensor_thread");
    if (IS_ERR(task)) {
        printk("Error creating sensor thread.\n");
        close_socket();
        return PTR_ERR(task);
    }

    return 0;
}

static int driver_release(struct inode *inode, struct file *file) {
    device_open--;
    module_put(THIS_MODULE);

    // Stop the sensor thread and close the socket
    if (task) {
        kthread_stop(task);
    }
    close_socket();

    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = driver_open,
    .release = driver_release
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
        close_socket();
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
