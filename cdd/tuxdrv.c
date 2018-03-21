#include <linux/module.h> /* for modules */
#include <linux/fs.h> /* file_operations */
#include <linux/uaccess.h> /* copy_(to,from)_user */
#include <linux/init.h> /* module_init, module_exit */
#include <linux/slab.h> /* kmalloc */
#include <linux/cdev.h> /* cdev utilities */


//Function Declarations
static int __init my_init(void);
static void __exit my_exit(void);
static int tux_open(struct inode *inode, struct file *file);
static int tux_rls(struct inode *inode, struct file *file);
static ssize_t tux_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos);
static ssize_t tux_write(struct file *file, const char __user * buf, size_t lbuf, loff_t * ppos);

static char *ramdisk;
#define BUF_LEN 100
#define MYDEV_NAME "tuxdrv"

static dev_t first;
static unsigned int count = 1;
static int Major = 700, Minor = 0;
static struct cdev *my_cdev;

static const struct file_operations mycdrv_fops = {
	//.owner = THIS_MODULE,
	.read = tux_read,
	.write = tux_write,
	.open = tux_open,
	.release = tux_rls,
};

static int __init my_init(void)
{
	ramdisk = kmalloc(BUF_LEN, GFP_KERNEL);
	first = MKDEV(Major, Minor);
	register_chrdev_region(first, count, MYDEV_NAME);
	my_cdev = cdev_alloc();
	cdev_init(my_cdev, &mycdrv_fops);
	cdev_add(my_cdev, first, count);
	printk(KERN_ALERT "Succeeded in registering character device %s\n", MYDEV_NAME);
	return 0;
}

static void __exit my_exit(void)
{
	cdev_del(my_cdev);
	unregister_chrdev_region(first, count);
	printk(KERN_ALERT "device unregistered\n");
	kfree(ramdisk);
}
module_init(my_init);
module_exit(my_exit);

static int tux_open(struct inode *inode, struct file *file)
{
	printk(KERN_ALERT " Open Dev: %s:\n", MYDEV_NAME);
	return 0;
}

static int tux_rls(struct inode *inode, struct file *file)
{
	printk(" Clsoe device: %s:\n", MYDEV_NAME);
	return 0;
}

static ssize_t tux_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{
	int nbytes;
	if ((lbuf + *ppos) > BUF_LEN) 
	{
		pr_info("trying to read past end of device,"
			"aborting because this is just a stub!\n");
		return 0;
	}
	nbytes = lbuf - copy_to_user(buf, ramdisk + *ppos, lbuf);
	*ppos += nbytes;
	printk(KERN_ALERT "READING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
	return nbytes;
}

static ssize_t tux_write(struct file *file, const char __user * buf, size_t lbuf, loff_t * ppos)
{
	int nbytes;
	if ((lbuf + *ppos) > BUF_LEN) 
	{
		pr_info("trying to read past end of device,"
			"aborting because this is just a stub!\n");
		return 0;
	}
	nbytes = lbuf - copy_from_user(ramdisk + *ppos, buf, lbuf);
	*ppos += nbytes;
	pr_info("\n WRITING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
	return nbytes;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FAN");

