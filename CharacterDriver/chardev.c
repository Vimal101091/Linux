#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define MAX_DEVICES 2
#define BUFFER_SIZE 1024 // Size of the device buffer

int base_minor = 0;
char *device_name = "mychardev";
dev_t devicenumber;
static struct device *dev;

static struct class *class = NULL;
static struct cdev mycdev[MAX_DEVICES];
static char *device_buffer[MAX_DEVICES]; // Device buffers for each device

// File operations prototypes
static int my_open(struct inode *inode, struct file *file);
static int my_release(struct inode *inode, struct file *file);
static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *offset);
static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *offset);
static loff_t my_llseek(struct file *file, loff_t offset, int whence);

// File operations structure
static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
    .llseek = my_llseek,
};

// Open function
static int my_open(struct inode *inode, struct file *file)
{
    file->private_data = (void *)(uintptr_t)iminor(inode);
    pr_info("Device %d opened\n", iminor(inode));
    return 0;
}

// Release function
static int my_release(struct inode *inode, struct file *file)
{
    pr_info("Device %d closed\n", iminor(inode));
    return 0;
}

// Read function
static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    int minor = (uintptr_t)file->private_data;
    char *buffer = device_buffer[minor];
    size_t bytes_to_read = min(count, BUFFER_SIZE - (size_t)*offset);

    if (*offset >= BUFFER_SIZE)
        return 0;

    if (copy_to_user(buf, buffer + *offset, bytes_to_read))
        return -EFAULT;

    *offset += bytes_to_read;
    pr_info("Read %zu bytes from device %d\n", bytes_to_read, minor);
    return bytes_to_read;
}

// Write function
static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    int minor = (uintptr_t)file->private_data;
    char *buffer = device_buffer[minor];
    size_t bytes_to_write = min(count, BUFFER_SIZE - (size_t)*offset);

    if (*offset >= BUFFER_SIZE)
        return -ENOSPC;

    if (copy_from_user(buffer + *offset, buf, bytes_to_write))
        return -EFAULT;

    *offset += bytes_to_write;
    pr_info("Wrote %zu bytes to device %d\n", bytes_to_write, minor);
    return bytes_to_write;
}

// llseek function
static loff_t my_llseek(struct file *file, loff_t offset, int whence)
{
    loff_t new_pos;

    switch (whence) {
    case SEEK_SET:
        new_pos = offset;
        break;
    case SEEK_CUR:
        new_pos = file->f_pos + offset;
        break;
    case SEEK_END:
        new_pos = BUFFER_SIZE + offset;
        break;
    default:
        return -EINVAL;
    }

    if (new_pos < 0 || new_pos > BUFFER_SIZE)
        return -EINVAL;

    file->f_pos = new_pos;
    pr_info("Seek to position %lld\n", new_pos);
    return new_pos;
}

// Device cleanup function
static void cleanup_devices(int devices_created)
{
    int i;

    for (i = 0; i < devices_created; i++) {
        device_destroy(class, MKDEV(MAJOR(devicenumber), i));
        cdev_del(&mycdev[i]);
        kfree(device_buffer[i]);
    }

    if (class)
        class_destroy(class);

    unregister_chrdev_region(devicenumber, MAX_DEVICES);
}

// Module init function
static int __init mymodule_init(void)
{
    int ret, i;

    // Allocate device numbers
    ret = alloc_chrdev_region(&devicenumber, base_minor, MAX_DEVICES, device_name);
    if (ret) {
        pr_err("Failed to allocate device numbers\n");
        return ret;
    }

    // Create device class
    class = class_create(THIS_MODULE, "myclass");
    if (IS_ERR(class)) {
        pr_err("Failed to create class\n");
        unregister_chrdev_region(devicenumber, MAX_DEVICES);
        return PTR_ERR(class);
    }

    // Initialize devices
    for (i = 0; i < MAX_DEVICES; i++) {
        // Allocate memory for device buffer
        device_buffer[i] = kzalloc(BUFFER_SIZE, GFP_KERNEL);
        if (!device_buffer[i]) {
            pr_err("Failed to allocate memory for device %d\n", i);
            ret = -ENOMEM;
            cleanup_devices(i); // Clean up already initialized devices
            return ret;
        }

        // Initialize and add cdev
        // Create device file
        dev = device_create(class, NULL, MKDEV(MAJOR(devicenumber), i), NULL, "mychardev%d", i);
        
        // Check if device_create was successful
        if (IS_ERR(dev)) {
            pr_err("Failed to create device %d\n", i);
            cdev_del(&mycdev[i]);
            kfree(device_buffer[i]);
            cleanup_devices(i);
            return PTR_ERR(dev);
        }
        
        cdev_init(&mycdev[i], &fops);
        
        ret = cdev_add(&mycdev[i], MKDEV(MAJOR(devicenumber), i), 1);
        if (ret) {
            pr_err("Failed to add cdev %d\n", i);
            kfree(device_buffer[i]);
            cleanup_devices(i);
            return ret;
        }

        

        
    }

    pr_info("Module loaded successfully\n");
    return 0;
}

// Module exit function
static void __exit mymodule_exit(void)
{
    cleanup_devices(MAX_DEVICES);
    pr_info("Module unloaded successfully\n");
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vimalsi");
MODULE_DESCRIPTION("Multi-Device Char Driver with Read/Write/Seek");


