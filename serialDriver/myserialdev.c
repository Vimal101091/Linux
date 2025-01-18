#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/io.h>

#define SERIAL_PORT_BASE 0x3F8 // Base address for COM1
#define DRIVER_NAME "myserialdev"
#define DEVICE_NAME "myserialdev"
#define MAX_STRING_LEN 256

static dev_t dev_num;
static struct cdev serial_cdev;
static struct class *serial_class;

static int serial_open(struct inode *inode, struct file *file)
{
    pr_info(DRIVER_NAME ": Device opened\n");
    return 0;
}

static int serial_close(struct inode *inode, struct file *file)
{
    pr_info(DRIVER_NAME ": Device closed\n");
    return 0;
}

static ssize_t serial_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    char kernel_buf[MAX_STRING_LEN];
    size_t i;

    if (count == 0)
        return 0;

    if (count > MAX_STRING_LEN - 1)
        count = MAX_STRING_LEN - 1;

    if (copy_from_user(kernel_buf, buf, count)) {
        return -EFAULT;
    }

    kernel_buf[count] = '\0'; // Null-terminate the string

    for (i = 0; i < count; i++) {
        // Wait for the transmitter holding register to be empty
        while (!(inb(SERIAL_PORT_BASE + 5) & 0x20));

        // Write each character to the serial port
        outb(kernel_buf[i], SERIAL_PORT_BASE);
    }

    pr_info(DRIVER_NAME ": Wrote string '%s' to serial port\n", kernel_buf);
    return count; // Indicate the number of bytes written
}

static struct file_operations serial_fops = {
    .owner = THIS_MODULE,
    .open = serial_open,
    .release = serial_close,
    .write = serial_write,
};

static int __init serial_init(void)
{
    int ret;

    // Allocate device number
    ret = alloc_chrdev_region(&dev_num, 0, 1, DRIVER_NAME);
    if (ret < 0) {
        pr_err(DRIVER_NAME ": Failed to allocate device number\n");
        return ret;
    }

    // Initialize and add cdev
    cdev_init(&serial_cdev, &serial_fops);
    ret = cdev_add(&serial_cdev, dev_num, 1);
    if (ret < 0) {
        pr_err(DRIVER_NAME ": Failed to add cdev\n");
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    // Create device class
    serial_class = class_create(THIS_MODULE, DRIVER_NAME);
    if (IS_ERR(serial_class)) {
        pr_err(DRIVER_NAME ": Failed to create class\n");
        cdev_del(&serial_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(serial_class);
    }

    // Automatically create the device node
    if (!device_create(serial_class, NULL, dev_num, NULL, DEVICE_NAME)) {
        pr_err(DRIVER_NAME ": Failed to create device\n");
        class_destroy(serial_class);
        cdev_del(&serial_cdev);
        unregister_chrdev_region(dev_num, 1);
        return -ENOMEM;
    }

    pr_info(DRIVER_NAME ": Module loaded, device created at /dev/%s\n", DEVICE_NAME);
    return 0;
}

static void __exit serial_exit(void)
{
    device_destroy(serial_class, dev_num);
    class_destroy(serial_class);
    cdev_del(&serial_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info(DRIVER_NAME ": Module unloaded, device removed\n");
}

module_init(serial_init);
module_exit(serial_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("VSI");
MODULE_DESCRIPTION("A simple serial port string writer driver");
