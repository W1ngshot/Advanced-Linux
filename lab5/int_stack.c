#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/usb.h>
#include <linux/device.h>

#define DEVICE_NAME "int_stack"
#define STACK_IOCTL_SET_SIZE _IOW('k', 1, int)
#define USB_MANUFACTURER_ID 0x1005
#define USB_ID 0xb113

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kirill Samsonov");
MODULE_DESCRIPTION("A kernel-based integer stack character driver with USB key");

struct int_stack_dev {
    int *buffer;
    int top;
    int max_size;
    struct mutex lock;
    struct cdev cdev;
    dev_t dev_num;
    struct class *cls;
};

static struct int_stack_dev *stack_dev;

static int stack_open(struct inode *inode, struct file *file) {
    pr_info("int_stack: Device opened\n");
    return 0;
}

static int stack_release(struct inode *inode, struct file *file) {
    pr_info("int_stack: Device closed\n");
    return 0;
}

static ssize_t stack_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    int val;
    ssize_t ret = 0;
    if (count < sizeof(int)) return -EINVAL;

    mutex_lock(&stack_dev->lock);
    if (stack_dev->top <= 0) {
        ret = 0;
    } else {
        stack_dev->top--;
        val = stack_dev->buffer[stack_dev->top];
        if (copy_to_user(buf, &val, sizeof(int))) {
            ret = -EFAULT;
        } else {
            ret = sizeof(int);
        }
    }
    mutex_unlock(&stack_dev->lock);
    return ret;
}

static ssize_t stack_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    int val;
    ssize_t ret = 0;
    if (count < sizeof(int)) return -EINVAL;

    mutex_lock(&stack_dev->lock);
    if (stack_dev->top >= stack_dev->max_size) {
        ret = -ERANGE;
    } else {
        if (copy_from_user(&val, buf, sizeof(int))) {
            ret = -EFAULT;
        } else {
            stack_dev->buffer[stack_dev->top] = val;
            stack_dev->top++;
            ret = sizeof(int);
        }
    }
    mutex_unlock(&stack_dev->lock);
    return ret;
}

static long stack_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int new_size;
    int *new_buffer;
    if (cmd != STACK_IOCTL_SET_SIZE) return -ENOTTY;

    if (copy_from_user(&new_size, (int __user *)arg, sizeof(int))) return -EFAULT;
    if (new_size <= 0) return -EINVAL;

    mutex_lock(&stack_dev->lock);
    new_buffer = kmalloc_array(new_size, sizeof(int), GFP_KERNEL);
    if (!new_buffer) {
        mutex_unlock(&stack_dev->lock);
        return -ENOMEM;
    }
    kfree(stack_dev->buffer);
    stack_dev->buffer = new_buffer;
    stack_dev->max_size = new_size;
    stack_dev->top = 0;
    mutex_unlock(&stack_dev->lock);
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = stack_read,
    .write = stack_write,
    .unlocked_ioctl = stack_ioctl,
    .open = stack_open,
    .release = stack_release,
};

static int usb_stack_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    int ret;
    pr_info("int_stack: USB Key inserted!\n");
    cdev_init(&stack_dev->cdev, &fops);
    ret = cdev_add(&stack_dev->cdev, stack_dev->dev_num, 1);
    if (ret < 0) return ret;

    stack_dev->cls = class_create("int_stack_class");
    if (IS_ERR(stack_dev->cls)) {
        cdev_del(&stack_dev->cdev);
        return PTR_ERR(stack_dev->cls);
    }

    device_create(stack_dev->cls, NULL, stack_dev->dev_num, NULL, DEVICE_NAME);
    return 0;
}

static void usb_stack_disconnect(struct usb_interface *interface) {
    device_destroy(stack_dev->cls, stack_dev->dev_num);
    class_destroy(stack_dev->cls);
    cdev_del(&stack_dev->cdev);
    pr_info("int_stack: USB Key removed!\n");
}

static struct usb_device_id usb_stack_table[] = {
    { USB_DEVICE(USB_MANUFACTURER_ID, USB_ID) },
    { }
};
MODULE_DEVICE_TABLE(usb, usb_stack_table);

static struct usb_driver stack_usb_driver = {
    .name = "int_stack_usb_driver",
    .probe = usb_stack_probe,
    .disconnect = usb_stack_disconnect,
    .id_table = usb_stack_table,
};

static int __init int_stack_init(void) {
    int ret;
    stack_dev = kzalloc(sizeof(struct int_stack_dev), GFP_KERNEL);
    if (!stack_dev) return -ENOMEM;

    ret = alloc_chrdev_region(&stack_dev->dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) { kfree(stack_dev); return ret; }

    mutex_init(&stack_dev->lock);
    stack_dev->max_size = 10;
    stack_dev->buffer = kmalloc_array(stack_dev->max_size, sizeof(int), GFP_KERNEL);

    ret = usb_register(&stack_usb_driver);
    if (ret) {
        unregister_chrdev_region(stack_dev->dev_num, 1);
        if (stack_dev->buffer) kfree(stack_dev->buffer);
        kfree(stack_dev);
        return ret;
    }
    return 0;
}

static void __exit int_stack_exit(void) {
    usb_deregister(&stack_usb_driver);
    unregister_chrdev_region(stack_dev->dev_num, 1);
    if (stack_dev->buffer) kfree(stack_dev->buffer);
    kfree(stack_dev);
    pr_info("int_stack: Module unloaded\n");
}

module_init(int_stack_init);
module_exit(int_stack_exit);
