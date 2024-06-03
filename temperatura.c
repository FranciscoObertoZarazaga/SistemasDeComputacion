#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <linux/inet.h>  // para in4_pton

#define DRIVER_NAME "bh1750_driver"
#define DRIVER_CLASS "BH1750Class"
#define I2C_BUS_AVAILABLE 1
#define SLAVE_DEVICE_NAME "BH1750"
#define BH1750_SLAVE_ADDRESS 0x23

#define SERVER_PORT 8001
#define SERVER_ADDR "127.0.0.1"

static dev_t my_device_nr;
static struct class *my_class;
static struct cdev my_device;

static struct i2c_adapter *bh1750_i2c_adapter = NULL;
static struct i2c_client *bh1750_i2c_client = NULL;
static struct socket *sock;
static struct task_struct *task;
static int device_open = 0;

static int setup_socket(void);
static void close_socket(void);
static int send_data_to_server(const char *data);

// I2C Board Info
static struct i2c_board_info bh1750_i2c_board_info = {
    I2C_BOARD_INFO(SLAVE_DEVICE_NAME, BH1750_SLAVE_ADDRESS)
};

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

static int bh1750_read_light(void) {
    int ret;
    char buf[2] = {0};
    int lux_value;

    ret = i2c_master_recv(bh1750_i2c_client, buf, 2);
    if (ret < 0) {
        printk(KERN_ERR "Failed to read data from BH1750 sensor\n");
        return ret;
    }

    lux_value = (buf[0] << 8) | buf[1];
    return lux_value;
}

static int sensor_thread(void *data) {
    while (!kthread_should_stop()) {
        char light_data[16];
        int lux_value = bh1750_read_light();

        if (lux_value >= 0) {
            snprintf(light_data, sizeof(light_data), "Light: %d\n", lux_value);
            send_data_to_server(light_data);
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
    printk("Initializing BH1750 driver...\n");

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

    // Initialize I2C
    bh1750_i2c_adapter = i2c_get_adapter(1);  // Cambia a 1 si estás usando i2c-1
    if (bh1750_i2c_adapter != NULL) {
        bh1750_i2c_client = i2c_new_client_device(bh1750_i2c_adapter, &bh1750_i2c_board_info);
        if (bh1750_i2c_client != NULL) {
            printk("BH1750 I2C client created\n");
        } else {
            printk("Failed to create BH1750 I2C client\n");
            goto AddError;
        }
        i2c_put_adapter(bh1750_i2c_adapter);
    } else {
        printk("Failed to get I2C adapter\n");
        goto AddError;
    }

    printk("BH1750 driver initialized.\n");
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
    if (bh1750_i2c_client) {
        i2c_unregister_device(bh1750_i2c_client);
    }
    cdev_del(&my_device);
    device_destroy(my_class, my_device_nr);
    class_destroy(my_class);
    unregister_chrdev_region(my_device_nr, 1);
    printk("BH1750 driver removed.\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GRUPO_LIPCEN_OBERTO_FERNANDEZ");
MODULE_DESCRIPTION("Driver de kernel para BH1750");
