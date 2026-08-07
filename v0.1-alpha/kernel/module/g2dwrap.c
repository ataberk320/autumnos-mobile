#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/cdev.h>

#define DEV_NAME "g2d"
#define CLS_NAME "sunxi_g2d"

#define SUNXI_G2D_BASE 0x01480000UL
#define SUNXI_G2D_SIZE 0x000BFFFFUL

#define G2D_CTL          0x0000
#define G2D_STATUS       0x0004
#define G2D_INT_EN       0x0008
#define G2D_INT_STATUS   0x000C
#define G2D_SRC_CADDR0   0x0040
#define G2D_SRC_CADDR1   0x0024
#define G2D_DST_CADDR0   0x0200
#define G2D_SRC_PITCH    0x004C

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Autix (ataberk320)");
MODULE_DESCRIPTION("Allwinner Technology G2D Hardware Accelerator Wrapper Activator.");
MODULE_VERSION("0.1");

static int major_num;
static struct class *g2d_cls = NULL;
static struct cdev g2d_cdev;
static void __iomem *g2d_base_addr = NULL;

#define G2D_IOC_MAGIC 'g'
#define G2D_CMD_BITBLT _IOW(G2D_IOC_MAGIC, 0x40, unsigned long)


typedef struct {
	uintptr_t addr;
	int w;
	int h;
	int pitch;
	int fmt;
} wrapper_g2d_image_t;

typedef struct {
	wrapper_g2d_image_t src;
	wrapper_g2d_image_t dst;
	int x;
	int y;
	int rotate;
	uint8_t alpha;
	uint8_t reserved[3];
} wrapper_g2d_blt_t;

static long gdioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	uint32_t ctl_val;
	if(!g2d_base_addr) return -EFAULT;

	switch (cmd) {
		case G2D_CMD_BITBLT: {
			wrapper_g2d_blt_t blt;
			if (copy_from_user(&blt, (void __user *)arg, sizeof(wrapper_g2d_blt_t))) {
				return -EFAULT;
			}
			
			writel(blt.src.addr, g2d_base_addr + G2D_SRC_CADDR0);
			writel(blt.src.pitch, g2d_base_addr + G2D_SRC_PITCH);
			writel(blt.dst.addr, g2d_base_addr + G2D_DST_CADDR0);

			ctl_val = readl(g2d_base_addr + G2D_CTL);
            		writel(ctl_val | 0x1, g2d_base_addr + G2D_CTL);

            		while (readl(g2d_base_addr + G2D_STATUS) & 0x1) {
                		cpu_relax();
            		}

            		break;
        	}
        	default:
            		return -EINVAL;
    	}
	return 0;
}

static int gdopen(struct inode *inodep, struct file *filep) {
	return 0;
}

static int gdrel(struct inode *inodep, struct file *filep) {
	return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = gdopen,
    .release = gdrel,
    .unlocked_ioctl = gdioctl,
};

static int __init gdinit(void) {
	dev_t dev;
	if (alloc_chrdev_region(&dev, 0, 1, DEV_NAME) < 0) return -1;
	major_num = MAJOR(dev);

	cdev_init(&g2d_cdev, &fops);
	if (cdev_add(&g2d_cdev, dev, 1) < 0) {
		unregister_chrdev_region(dev, 1);
		return -1;
	}

	g2d_cls = class_create(THIS_MODULE, CLS_NAME);
	if (IS_ERR(g2d_cls)) {
		cdev_del(&g2d_cdev);
		unregister_chrdev_region(dev, 1);
		return PTR_ERR(g2d_cls);
	}

	if (IS_ERR(device_create(g2d_cls, NULL, dev, NULL, DEV_NAME))) {
		class_destroy(g2d_cls);
		cdev_del(&g2d_cdev);
		unregister_chrdev_region(dev, 1);
		return -1;
	}

	g2d_base_addr = ioremap(SUNXI_G2D_BASE, SUNXI_G2D_SIZE);
    	if (!g2d_base_addr) {
        	device_destroy(g2d_cls, dev);
        	class_destroy(g2d_cls);
        	cdev_del(&g2d_cdev);
        	unregister_chrdev_region(dev, 1);
        	return -ENOMEM;
    	}

    return 0;
}

static void __exit gdexit(void) {
    dev_t dev = MKDEV(major_num, 0);
    if (g2d_base_addr) iounmap(g2d_base_addr);
    device_destroy(g2d_cls, dev);
    class_destroy(g2d_cls);
    cdev_del(&g2d_cdev);
    unregister_chrdev_region(dev, 1);
}

module_init(gdinit);
module_exit(gdexit);
