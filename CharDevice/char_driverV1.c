#include <linux/module.h> /* for mudules */
#include <linux/fs.h>  /* file operations */
#include <linux/uaccess.h> /* copy_(to, from)_user */
#include <linux/init.h> /* module init, module exit */
#include <linux/slab.h> /* kmalloc */
#include <linux/cdev.h> /* cdev utilities */
#include <linux/semaphore.h>

#define MYDEV_NAME "mycdrv0"
#define ramdisk_size (size_t) (16 * PAGE_SIZE) // ramdisk size 

//NUM_DEVICES defaults to 3 unless specified during insmod
static int NUM_DEVICES = 3;

module_param(NUM_DEVICES, int, S_IRUGO);

#define CDRV_IOC_MAGIC 'Z'
#define ASP_CLEAR_BUF _IOW(CDRV_IOC_MAGIC, 1, int)
#define ERROR -1

/********************************************************
 * Declaration of functions
*********************************************************/
static int mydrv_open(struct inode *, struct file *);
static int mydrv_rls(struct inode *, struct file *);
static ssize_t mydrv_read(struct file *, char *, size_t, loff_t *);
static ssize_t mydrv_write(struct file *, const char *, size_t, loff_t *);
static void free_kalloc(void);

static const struct file_operations mycdrv_fops = {
    .owner = THIS_MODULE,
    .read = mydrv_read,
    .write = mydrv_write,
    .open = mydrv_open,
    .release = mydrv_rls
}; 

static struct ASP_mycdrv {
	struct cdev *dev;
	char *ramdisk;
    struct class *class;
	struct semaphore sem;
	int DevNo;
    dev_t first;
	// any other field you may want to add
} asp_mycdrv;

static int __init my_init(void)
{
    int i=0;

    asp_mycdrv.dev = NULL;
    asp_mycdrv.ramdisk = NULL;
    asp_mycdrv.class = NULL;

    //module_param(NUM_DEVICES, int, S_IRUGO);
   
    
    //  allocate a range of character device numbers
    if (alloc_chrdev_region(&asp_mycdrv.first, 0, NUM_DEVICES, MYDEV_NAME) < 0)
    {
        printk(KERN_ALERT "\n%s: Failed to allocate a major number", MYDEV_NAME);
        return ERROR;
    }

    for (i=0; i<NUM_DEVICES; ++i)
    {

        //  allocate a device class
        if ((asp_mycdrv.class = class_create(THIS_MODULE, MYDEV_NAME)) == NULL)
        {
            printk(KERN_ALERT "\n%s: failed to allocate class", MYDEV_NAME);
            free_kalloc();
            return ERROR;
        }

        dev_t devno = MKDEV(asp_mycdrv.first, i);

        //  allocate a device file
        if (device_create(asp_mycdrv.class, NULL, devno, NULL, MYDEV_NAME"%d", i) == NULL )
        {
            printk(KERN_ALERT "\n%s: failed to allocate device file", MYDEV_NAME);
            free_kalloc();
            return ERROR;
        }

        //  allocate a character device structure
        asp_mycdrv.dev = cdev_alloc();
        asp_mycdrv.dev->ops = &mycdrv_fops;
        asp_mycdrv.dev->owner = THIS_MODULE;

        //  allocate a buffer of size ramdisk_size
        if ((asp_mycdrv.ramdisk = kmalloc(ramdisk_size, GFP_KERNEL)) == NULL)
        {
            printk(KERN_ALERT "\n%s: failed to allocate buffer", MYDEV_NAME);
            free_kalloc();
            return ERROR;
        }
        //  add device to the kernel
        if (cdev_add(asp_mycdrv.dev, devno, 1) == ERROR)
        {
            printk(KERN_ALERT "\n%s: unable to add char device", MYDEV_NAME);
            free_kalloc();
            return ERROR;
        }
    }
    //  initialize semaphores
    sema_init(&asp_mycdrv.sem, 1);

    printk(KERN_ALERT "\n%s: loaded module", MYDEV_NAME);
    return 0;
}

static void __exit my_exit(void) /* destructor */
{
    free_kalloc();
    //  unregister_chrdev_region(first, NUM_DEVICES);
    printk(KERN_ALERT "\n%s: unloaded module", MYDEV_NAME);
}

static ssize_t mydrv_read(struct file *file, char *dst, size_t count,
                        loff_t *f_offset)
{
    printk(KERN_ALERT "\n%s: reading from device", MYDEV_NAME);
    return copy_to_user(dst, asp_mycdrv.ramdisk, count);
}

static ssize_t mydrv_write(struct file *file, const char *src, size_t count,
                            loff_t *f_offset)
{
    // return an error code when device is already open for writing
    if (down_interruptible(&asp_mycdrv.sem))
        return -ERESTARTSYS;
    printk(KERN_ALERT "\n%s: writing to device", MYDEV_NAME);

    if(copy_from_user(asp_mycdrv.ramdisk, src, count) != 0)
        return -EFAULT;

    //  free semaphore
    up(&asp_mycdrv.sem);
    return 0;
}

static int mydrv_open(struct inode *inode, struct file *file)
{
    printk(KERN_ALERT "\n%s: opened device", MYDEV_NAME);
    return 0;
}

static int mydrv_rls(struct inode *inode, struct file *file)
{
    printk(KERN_ALERT "\n%s: device close", MYDEV_NAME);
    return 0;
}

/********************************
 * Frees the dynamic memory allocations.
 * 
*************************************/
void free_kalloc()
{
    int i;

    if (asp_mycdrv.ramdisk)
        kfree(asp_mycdrv.ramdisk);
    if(asp_mycdrv.dev)
        cdev_del(asp_mycdrv.dev);
    for (i=0; i<NUM_DEVICES; ++i)
    {
        if(asp_mycdrv.class && asp_mycdrv.first)
        {
            device_destroy(asp_mycdrv.class, MKDEV(asp_mycdrv.first, i));
            class_destroy(asp_mycdrv.class);  
        }
    }
    unregister_chrdev_region(MKDEV(asp_mycdrv.first, 0), NUM_DEVICES);
}
module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("user");
MODULE_LICENSE("GPL v2");
