#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h> // copy_to_user, copy_from_user

#define DEVICE_NAME "mychardevice"
#define BUF_LEN 80

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tinge");
MODULE_DESCRIPTION("A simple char device driver");

static int major;
static char msg[BUF_LEN];
static int msg_len = 0;

// open
static int dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "mychardev: Device opened\n");
    return 0;
}

// release / close
static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "mychardev: Device closed\n");
    return 0;
}

// read
static ssize_t dev_read(struct file *filp, char __user *buffer, size_t len, loff_t *offset)
{
    int bytes_to_copy = msg_len - *offset;
    if (bytes_to_copy <= 0) return 0; // EOF

    if (len < bytes_to_copy) bytes_to_copy = len;

    if (copy_to_user(buffer, msg + *offset, bytes_to_copy) != 0)
        return -EFAULT;

    *offset += bytes_to_copy;
    printk(KERN_INFO "mychardev: Read %d bytes\n", bytes_to_copy);
    return bytes_to_copy;
}

// write
static ssize_t dev_write(struct file *filp, const char __user *buffer, size_t len, loff_t *offset)
{
    if (len > BUF_LEN - 1) len = BUF_LEN - 1;

    if (copy_from_user(msg, buffer, len) != 0)
        return -EFAULT;

    msg[len] = '\0';
    msg_len = len;
    printk(KERN_INFO "mychardev: Written %zu bytes\n", len);
    return len;
}

// file operations
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
    .write = dev_write,
    .open = dev_open,
    .release = dev_release,
};

// module init
static int __init chardev_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "mychardev: Failed to register device\n");
        return major;
    }
    printk(KERN_INFO "mychardev: Registered with major number %d\n", major);
    return 0;
}

// module exit
static void __exit chardev_exit(void)
{
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "mychardev: Unregistered device\n");
}

module_init(chardev_init);
module_exit(chardev_exit);
