#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/sched.h>

#define DEV_NAME "tabldev"
#define CLS_NAME "atabl"
#define SUCCESS 0

#define IOC_M 'a'
#define IOCTBLREGSRV _IOW(IOC_M, 1, unsigned long)
#define IOCTBLGETSRV _IOR(IOC_M, 2, unsigned long)

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("AutumnOS TableLoader driver");

static int major_num;
static struct class *tabl_cls = NULL;
static struct device *tabl_dev = NULL;

static unsigned long reg_srv_data = 0;
static pid_t session_pid = 0

static int dopen(struct inode *inode, struct file *file) {
	return SUCCESS;
}

static int drel(struct inode *inode, struct file *file) {
	return SUCCESS;
}


static int dread(struct file *filp, char __user *buf, size_t len, loff_t *offset) {
	return 0;
}

static ssize_t dwrite(struct file *filp, const char __user *buffer, size_t length, loff_t *offset) {
    return 0;
}

static long dioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	switch (cmd) {
		case IOCTBLREGSRV:
			if (session_pid == 0) {
				session_pid = task_pid_nr(current);
			}

			if (task_pid_nr(current) != session_pid) {
				return -EPERM;
			}

			reg_srv_data = arg;
			break;
		case IOCTBLGETSRV:
			if (copy_to_user((unsigned long __user *)arg, &reg_srv_data, sizeof(unsigned long))) {
				return -EFAULT;
			}
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = dopen,
	.release = drel,
	.read = dread,
	.write = dwrite,
	.unlocked_ioctl = dioctl,
};

static int __init devinit(void) {
	major_num = register_chrdev(0, DEV_NAME, &fops);
	if (major_num < 0) {
		return major_num;
	}

	tabl_cls = class_create(THIS_MODULE, CLS_NAME);
    	if (IS_ERR(tabl_cls)) {
        	unregister_chrdev(major_num, DEV_NAME);
        	return PTR_ERR(tabl_cls);
    	}

    	tabl_dev = device_create(tabl_cls, NULL, MKDEV(major_num, 0), NULL, DEVICE_NAME);
    	if (IS_ERR(tabl_dev)) {
        	class_destroy(tabl_cls);
        	unregister_chrdev(major_num, DEV_NAME);
        	return PTR_ERR(tabl_dev);
    	}
	return SUCCESS;
}

static void __exit devexit(void) {
    	device_destroy(tabl_cls, MKDEV(major_num, 0));
    	class_destroy(tabl_cls);
    	unregister_chrdev(major_num, DEV_NAME);
}

module_init(devinit);
module_exit(devexit);
