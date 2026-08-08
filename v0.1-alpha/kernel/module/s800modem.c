// SPDX-License-Identifier: GPL-2.0-only
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/delay.h> // msleep

#define DEV_NAME "simmod0"
#define CLS_NAME "sim800_class"

#define SIM_IOC_MAGIC 's'
#define IOCMODSCAN       _IO(SIM_IOC_MAGIC, 1)
#define IOCMODGETSTAT     _IOR(SIM_IOC_MAGIC, 2, int)
#define IOCMODSNDCMD   _IOW(SIM_IOC_MAGIC, 3, char*)
#define IOCMODRDRESP  _IOR(SIM_IOC_MAGIC, 4, char*)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Autix (ataberk320)");
MODULE_DESCRIPTION("SIM800L Direct Device driver.");
MODULE_VERSION("0.1");

static int major_num;
static struct class *sim_cls = NULL;
static struct cdev sim_cdev;
static int modem_ready = 0;
static int act_port_idx = -1;
static char path[32];

static int mdscan(void) {
	struct file *filp;
	char tty_p[32];
	char rx_buf[64];
	int i, bytes;

	for (i = 0; i < 8; i++) {
		snprintf(tty_p, sizeof(tty_p), "/dev/ttyS%d", i);
		
		filp = filp_open(tty_p, O_RDWR | O_NOCTTY | O_NONBLOCK, 0);
		if (IS_ERR(filp)) continue;

		if (filp->f_op && filp->f_op->write) {
			filp->f_op->write(filp, "AT\r\n", 4, &filp->f_pos);
		}

		msleep(100);

		memset(rx_buf, 0, sizeof(rx_buf));
		if (filp->f_op && filp->f_op->read) {
			bytes = filp->f_op->read(filp, rx_buf, sizeof(rx_buf) - 1, &filp->f_pos);
			if (bytes > 0 && strstr(rx_buf, "OK")) {
				act_port_idx = i;
                		snprintf(path, sizeof(path), "%s", tty_p);
                		filp_close(filp, NULL);
                		return 1;
            		}
        	}
        	filp_close(filp, NULL);
    	}
    	return 0;
}

static int mdwrite(const char *cmd) {
	struct file *filp;
	if (act_port_idx < 0) return -ENODEV;

	filp = filp_open(path, O_RDWR | O_NOCTTY | O_NONBLOCK, 0);
	if (IS_ERR(filp)) return PTR_ERR(filp);

	if (filp->f_op && filp->f_op->write) {
		filp->f_op->write(filp, cmd, strlen(cmd), &filp->f_pos);
    	}

    	filp_close(filp, NULL);
    	return 0;
}

static int mdread(char *buf, size_t len) {
	struct file *filp;
	int bytes = 0;
	if (act_port_idx < 0) return -ENODEV;

	filp = filp_open(path, O_RDWR | O_NOCTTY | O_NONBLOCK, 0);
    	if (IS_ERR(filp)) return PTR_ERR(filp);

    	msleep(100);
    	if (filp->f_op && filp->f_op->read) {
        	bytes = filp->f_op->read(filp, buf, len - 1, &filp->f_pos);
        	if (bytes > 0) buf[bytes] = '\0';
    	}

    	filp_close(filp, NULL);
    	return bytes;
}

static long mdioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	switch (cmd) {
		case IOCMODSCAN:
			modem_ready = mdscan();
			break;

		case IOCMODGETSTAT:
			if (copy_to_user((int __user *)arg, &modem_ready, sizeof(int))) {
				return -EFAULT;
			}
			break;

		case IOCMODSNDCMD: {
			char k_cmd[128];
			if (copy_from_user(k_cmd, (char __user *)arg, sizeof(k_cmd))) {
				return -EFAULT;
			}
			mdwrite(k_cmd);
			break;
		}

		case IOCMODRDRESP: {
			char k_buf[128];
			memset(k_buf, 0, sizeof(k_buf));
			mdread(k_buf, sizeof(k_buf));
			if (copy_to_user((char __user *)arg, k_buf, sizeof(k_buf))) {
                		return -EFAULT;
            		}
            		break;
		}

		default:
			return -EINVAL;
	}
	return 0;
}

static int mdopen(struct inode *inodep, struct file *filep) {
	return 0;
}

static int mdrel(struct inode *inodep, struct file *filep) { 
	return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = mdopen,
    .release = mdrel,
    .unlocked_ioctl = mdioctl,
};

static int __init mdinit(void) {
	dev_t dev;
    	if (alloc_chrdev_region(&dev, 0, 1, DEV_NAME) < 0) return -1;
    	major_num = MAJOR(dev);

    	cdev_init(&sim_cdev, &fops);
    	if (cdev_add(&sim_cdev, dev, 1) < 0) {
        	unregister_chrdev_region(dev, 1);
        	return -1;
    	}

    	sim_cls = class_create(THIS_MODULE, CLS_NAME);
    	if (IS_ERR(sim_cls)) {
        	cdev_del(&sim_cdev);
        	unregister_chrdev_region(dev, 1);
        	return PTR_ERR(sim_cls);
    	}

    	if (IS_ERR(device_create(sim_cls, NULL, dev, NULL, DEV_NAME))) {
        	class_destroy(sim_cls);
        	cdev_del(&sim_cdev);
        	unregister_chrdev_region(dev, 1);
        	return -1;
    	}

    return 0;
}

static void __exit mdexit(void) {
    dev_t dev = MKDEV(major_num, 0);
    device_destroy(sim_cls, dev);
    class_destroy(sim_cls);
    cdev_del(&sim_cdev);
    unregister_chrdev_region(dev, 1);
}

module_init(mdinit);
module_exit(mdexit);
