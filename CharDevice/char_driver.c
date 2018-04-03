#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

#define MYDEV_NAME "mycdrv"
/* Number of devices to create (default: cfake0 and cfake1) */
#define DEFAULT_NDEVICES 3    

/* Size of a buffer used for data storage */
#define MY_BUFFER_SIZE (size_t) (16 * PAGE_SIZE)

/* Maxumum length of a block that can be read or written in one operation */
#define MY_BLOCK_SIZE 512

#define ASP_IOCTL_MAXNR 0
#define CDRV_IOC_MAGIC 0x37
#define ASP_CLEAR_BUF _IO(CDRV_IOC_MAGIC, 0)


/* The structure to represent 'cfake' devices. 
 *  data - data buffer;
 *  buffer_size - size of the data buffer;
 *  block_size - maximum number of bytes that can be read or written 
 *    in one call;
 *  cfake_mutex - a mutex to protect the fields of this structure;
 *  cdev - ñharacter device structure.
 */
struct asp_mycdev {
	unsigned char *data;
	unsigned long buffer_size; 
	unsigned long block_size;  
	//struct mutex cfake_mutex; 
    struct semaphore sem;
	struct cdev cdev;
};
/* ================================================================ */

static unsigned int cfake_major = 0;
static struct asp_mycdev *cfake_devices = NULL;
static struct class *cfake_class = NULL;
/* ================================================================ */

/* Declaration of Functions */
static int mycdev_open(struct inode *, struct file *);
static ssize_t mycdev_read(struct file *, char __user *, size_t, loff_t *);
static int mycdev_rls(struct inode *, struct file *);
static ssize_t mycdev_write(struct file *, const char __user *, size_t, loff_t *);
static loff_t mycdev_llseek(struct file *, loff_t, int);
static int mycdev_construct_device(struct asp_mycdev *, int, struct class *);
static void mycdev_destroy_device(struct asp_mycdev *, int, struct class *);
static void mycdev_cleanup_module(int);
static long mycdev_ioctl(struct file *, unsigned int, unsigned long);

static const struct file_operations cfake_fops = {
	.owner =    THIS_MODULE,
	.read =     mycdev_read,
	.write =    mycdev_write,
	.open =     mycdev_open,
	.release =  mycdev_rls,
	.llseek =   mycdev_llseek,
    .unlocked_ioctl =   mycdev_ioctl,
};

/* parameters */
static int NUM_DEVICES = DEFAULT_NDEVICES;
static unsigned long cfake_buffer_size = MY_BUFFER_SIZE;
static unsigned long cfake_block_size = MY_BLOCK_SIZE;

module_param(NUM_DEVICES, int, S_IRUGO);
module_param(cfake_buffer_size, ulong, S_IRUGO);
module_param(cfake_block_size, ulong, S_IRUGO);


static int __init mycdev_init(void)
{
	int err = 0;
	int i = 0;
	int devices_to_destroy = 0;
	dev_t dev = 0;
	
	if (NUM_DEVICES <= 0)
	{
		printk(KERN_WARNING "[target] Invalid value of NUM_DEVICES: %d\n", 
			NUM_DEVICES);
		err = -EINVAL;
		return err;
	}
	
	/* Get a range of minor numbers (starting with 0) to work with */
	err = alloc_chrdev_region(&dev, 0, NUM_DEVICES, MYDEV_NAME);
	if (err < 0) {
		printk(KERN_WARNING "[target] alloc_chrdev_region() failed\n");
		return err;
	}
	cfake_major = MAJOR(dev);

	/* Create device class (before allocation of the array of devices) */
	cfake_class = class_create(THIS_MODULE, MYDEV_NAME);
	if (IS_ERR(cfake_class)) {
		err = PTR_ERR(cfake_class);
		goto fail;
	}
	
	/* Allocate the array of devices */
	cfake_devices = (struct asp_mycdev *)kzalloc(
		NUM_DEVICES * sizeof(struct asp_mycdev), 
		GFP_KERNEL);
	if (cfake_devices == NULL) {
		err = -ENOMEM;
		goto fail;
	}
	
	/* Construct devices */
	for (i = 0; i < NUM_DEVICES; ++i) {
		err = mycdev_construct_device(&cfake_devices[i], i, cfake_class);
		if (err) {
			devices_to_destroy = i;
			goto fail;
		}
	}
	return 0; /* success */

fail:
	mycdev_cleanup_module(devices_to_destroy);
	return err;
}

static void __exit mycdev_exit(void)
{
	mycdev_cleanup_module(NUM_DEVICES);
	return;
}

module_init(mycdev_init);
module_exit(mycdev_exit);

static int mycdev_open(struct inode *inode, struct file *filp)
{
	unsigned int mj = imajor(inode);
	unsigned int mn = iminor(inode);
	
	struct asp_mycdev *dev = NULL;
	
	if (mj != cfake_major || mn < 0 || mn >= NUM_DEVICES)
	{
		printk(KERN_WARNING "[target] "
			"No device found with minor=%d and major=%d\n", 
			mj, mn);
		return -ENODEV; /* No such device */
	}
	
	/* store a pointer to struct asp_mycdev here for other methods */
	dev = &cfake_devices[mn];
	filp->private_data = dev; 
	
	if (inode->i_cdev != &dev->cdev)
	{
		printk(KERN_WARNING "[target] open: internal error\n");
		return -ENODEV; /* No such device */
	}
	
	/* if opened the 1st time, allocate the buffer */
	if (dev->data == NULL)
	{
		dev->data = (unsigned char*)kzalloc(dev->buffer_size, GFP_KERNEL);
		if (dev->data == NULL)
		{
			printk(KERN_WARNING "[target] open(): out of memory\n");
			return -ENOMEM;
		}
	}
	return 0;
}

static int mycdev_rls(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t mycdev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct asp_mycdev *dev = (struct asp_mycdev *)filp->private_data;
	ssize_t retval = 0;
	
	//if (mutex_lock_killable(&dev->cfake_mutex))
    if (down_interruptible(&dev->sem))
		return -EINTR;
	
	if (*f_pos >= dev->buffer_size) /* EOF */
		goto out;
	
	if (*f_pos + count > dev->buffer_size)
		count = dev->buffer_size - *f_pos;
	
	if (count > dev->block_size)
		count = dev->block_size;
	
	if (copy_to_user(buf, &(dev->data[*f_pos]), count) != 0)
	{
		retval = -EFAULT;
		goto out;
	}
	
	*f_pos += count;
	retval = count;
	
out:
	//mutex_unlock(&dev->cfake_mutex);
    up(&dev->sem);
	return retval;
}
				
static ssize_t mycdev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct asp_mycdev *dev = (struct asp_mycdev *)filp->private_data;
	ssize_t retval = 0;
	
	//if (mutex_lock_killable(&dev->cfake_mutex))
    if (down_interruptible(&dev->sem))
		return -EINTR;
	
	if (*f_pos >= dev->buffer_size) {
	/* Writing beyond the end of the buffer is not allowed. */
		retval = -EINVAL;
		goto out;
	}
	
	if (*f_pos + count > dev->buffer_size)
		count = dev->buffer_size - *f_pos;
	
	if (count > dev->block_size)
		count = dev->block_size;
	
	if (copy_from_user(&(dev->data[*f_pos]), buf, count) != 0)
	{
		retval = -EFAULT;
		goto out;
	}
	
	*f_pos += count;
	retval = count;
	
out:
	//mutex_unlock(&dev->cfake_mutex);
    up(&dev->sem);
	return retval;
}

static loff_t mycdev_llseek(struct file *filp, loff_t off, int whence)
{
	struct asp_mycdev *dev = (struct asp_mycdev *)filp->private_data;
	loff_t newpos = 0;
	
	switch(whence) {
	  case 0: /* SEEK_SET */
		newpos = off;
		break;

	  case 1: /* SEEK_CUR */
		newpos = filp->f_pos + off;
		break;

	  case 2: /* SEEK_END */
		newpos = dev->buffer_size + off;
		break;

	  default: /* can't happen */
		return -EINVAL;
	}
	if (newpos < 0 || newpos > dev->buffer_size) 
		return -EINVAL;
	
	filp->f_pos = newpos;
	return newpos;
}

/* ================================================================ */
/* Setup and register the device with specific index (the index is also
 * the minor number of the device).
 * Device class should be created beforehand.
 */
static int mycdev_construct_device(struct asp_mycdev *dev, int minor, struct class *class)
{
	int err = 0;
	dev_t devno = MKDEV(cfake_major, minor);
	struct device *device = NULL;
	
	BUG_ON(dev == NULL || class == NULL);

	/* Memory is to be allocated when the device is opened the first time */
	dev->data = NULL;     
	dev->buffer_size = cfake_buffer_size;
	dev->block_size = cfake_block_size;
	sema_init(&dev->sem, 1);
	
	cdev_init(&dev->cdev, &cfake_fops);
	dev->cdev.owner = THIS_MODULE;
	
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
	{
		printk(KERN_WARNING "[target] Error %d while trying to add %s%d",
			err, MYDEV_NAME, minor);
		return err;
	}

	device = device_create(class, NULL, /* no parent device */ 
		devno, NULL, /* no additional data */
		MYDEV_NAME"%d", minor);

	if (IS_ERR(device)) {
		err = PTR_ERR(device);
		printk(KERN_WARNING "[target] Error %d while trying to create %s%d",
			err, MYDEV_NAME, minor);
		cdev_del(&dev->cdev);
		return err;
	}
	return 0;
}

/* Destroy the device and free its buffer */
static void mycdev_destroy_device(struct asp_mycdev *dev, int minor, struct class *class)
{
	BUG_ON(dev == NULL || class == NULL);
	device_destroy(class, MKDEV(cfake_major, minor));
	cdev_del(&dev->cdev);
	kfree(dev->data);
	//mutex_destroy(&dev->cfake_mutex);
	return;
}

/* ================================================================ */
static void mycdev_cleanup_module(int devices_to_destroy)
{
	int i;
	
	/* Get rid of character devices (if any exist) */
	if (cfake_devices) {
		for (i = 0; i < devices_to_destroy; ++i) {
			mycdev_destroy_device(&cfake_devices[i], i, cfake_class);
		}
		kfree(cfake_devices);
	}
	
	if (cfake_class)
		class_destroy(cfake_class);

	/* [NB] mycdev_cleanup_module is never called if alloc_chrdev_region()
	 * has failed. */
	unregister_chrdev_region(MKDEV(cfake_major, 0), NUM_DEVICES);
	return;
}


static long mycdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct asp_mycdev *dev = (struct asp_mycdev *)filp->private_data;

    long retval = -1;
	//struct asp_mycdev *mycdev = NULL;

	/* Extracts type and number bitfields;
	don't decode wrong commands; return -ENOTTY (Inappropriate IOCTL)  */
   	if(_IOC_TYPE(cmd) !=CDRV_IOC_MAGIC)
		return -ENOTTY;
	if(_IOC_NR(cmd) > ASP_IOCTL_MAXNR)
		return -ENOTTY;

	/* If everything is fine, extract the command and perform action */
	//mycdev = filp->private_data;

	/* Enter Critical Section */
	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	switch (cmd)
	{
		/* clear the ramdisk & seek to start of the file */
		case ASP_CLEAR_BUF:
			memset(dev->data, 0, dev->buffer_size);
			filp->f_pos = 0;
			//dev->devReset = true;
			retval = 1;
			break;

		/* the control is unlikely to come here after MAXNR check above */
		default:
			retval = -ENOTTY;
	}

	/* Exit Critical Section */
	up(&dev->sem);

	return retval;
}

MODULE_AUTHOR("FAN LU");
MODULE_LICENSE("GPL v2");
