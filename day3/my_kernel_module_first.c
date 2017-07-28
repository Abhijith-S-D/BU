#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h> /* For the character driver support */

int procmon_open(struct inode *pinode, struct file *pfile){
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	return 0;
}

ssize_t procmon_read(struct file *pfile, char __user *buffer,size_t length, loff_t *offset){
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	return 0;
}

ssize_t procmon_write(struct file *pfile,const char __user *buffer,size_t length, loff_t *offset){
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	return length;
}

int procmon_release(struct inode *pinode,struct file *pfile){
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	return 0;
}

/*To hold the file operations performed on this device */
struct file_operations my_procmon = {
	.owner	= THIS_MODULE,
	.open	= procmon_open,
	.read	= procmon_read,
	.write	= procmon_write,
	.release	= procmon_release,
};

__init int my_kernel_module_init(void){
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	
	/*Register with the kernel and indicate that we are registering a character device driver*/
	register_chrdev(164/*Major Number*/,
	"My procmon"/*Name of the driver*/,
	&my_procmon/*File Operations*/);
	return 0;
}

void my_kernel_module_exit(void){
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	/*Unregister the character device driver*/
	unregister_chrdev(164,"My procmon");
}

module_init(my_kernel_module_init);
module_exit(my_kernel_module_exit);
