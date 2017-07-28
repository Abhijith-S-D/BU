#include<linux/init.h>
#include<linux/module.h>
#include<linux/printk.h>
#include<linux/device.h>
#include<asm-generic/uaccess.h>
#include<linux/sched.h>
#include<linux/slab.h>
#include<linux/fs.h> /* For the character driver support */
#define  DEVICE_NAME "process_list"
#define  CLASS_NAME  "ebb"

MODULE_LICENSE("GPL");

static int    majorNumber=164;
static short size_of_message;
static int    numberOpens = 0;
static int count=0;
static int curr=0;
//static struct class*  ebbcharClass  = NULL;
//static struct device* ebbcharDevice = NULL;
static struct task_struct *task;
static struct task_struct** task_array;

struct mydata{
	int pid;
	int ppid;
	int cpu;
	long int state;
	int valid;
};
static struct mydata* message;
static char* msg;
int start_iterating(void){
	struct task_struct** temp;
	for_each_process(task){
		count++;
	}
	task_array=(struct task_struct**)kmalloc(count*sizeof(struct task_struct*),GFP_KERNEL);
	temp=task_array;
	for_each_process(task){
		//*temp=(struct task_struct*)kmalloc(count*sizeof(struct task_struct),GFP_KERNEL);
		*temp=task;
		temp++;
	}
	return 0;
}

int procmon_open(struct inode *pinode, struct file *pfile){
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	numberOpens++;
   	printk(KERN_INFO "process_list: Device has been opened %d time(s)\n", numberOpens);
	return 0;
}

ssize_t procmon_read(struct file *pfile, char __user *buffer,size_t length, loff_t *offset){
	int error_count = 0;
	if(curr<count){
		struct task_struct *curr_task=task_array[curr];
		curr++;
		printk(KERN_ALERT "Inside the %s function length=%d\n", __FUNCTION__,(int)length);
		if (message)
			kfree(message);
		message =(struct mydata*)kmalloc(sizeof(struct mydata), GFP_KERNEL);
		message->pid=curr_task->pid;
		message->ppid=curr_task->parent->pid;
		message->cpu=task_cpu(curr_task);
		message->state=curr_task->state;
		message->valid=1;
		//sprintf(message,"PID=%d PPID=%d CPU=%d STATE=%ld\n",curr_task->pid,curr_task->parent->pid,task_cpu(curr_task),curr_task->state);
		printk(KERN_INFO "PID=%d PPID=%d CPU=%d STATE=%ld\n",curr_task->pid,curr_task->parent->pid,task_cpu(curr_task),curr_task->state);
		size_of_message = sizeof(struct mydata);
		// copy_to_user has the format ( * to, *from, size) and returns 0 on success
		if(access_ok(VERIFY_WRITE,buffer,size_of_message)>0){
		error_count = copy_to_user(buffer, message, size_of_message);
		}else{
			printk(KERN_ALERT"sorry couldnt write to user space");
		}
		if (error_count!=0){
			printk(KERN_INFO "process_list: Failed to send %d characters to the user\n", error_count);
			return -EFAULT;
			}
	//flag=0;
	}else{
		if (message)
			kfree(message);
	
		message =(struct mydata*)kmalloc(sizeof(struct mydata), GFP_KERNEL);
		message->pid=0;
		message->ppid=0;
		message->cpu=0;
		message->state=0;
		message->valid=0;
		//printk(KERN_ALERT"%s\n",message);
		size_of_message = sizeof(struct mydata);
		// copy_to_user has the format ( * to, *from, size) and returns 0 on success
		if(access_ok(VERIFY_WRITE,buffer,size_of_message)>0){
		error_count = copy_to_user(buffer, message, size_of_message);
		}else{
			printk(KERN_ALERT"sorry couldnt write to user space");
		}
		if (error_count!=0){
			printk(KERN_INFO "process_list: Failed to send %d characters to the user\n", error_count);
			return -EFAULT;
			}
		//flag=0;
	}
	return 0;
}

ssize_t procmon_write(struct file *pfile,const char __user *buffer,size_t length, loff_t *offset){
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	if (msg)
		kfree(msg);

	msg = (char*)kmalloc(length, GFP_KERNEL);
	sprintf(msg, "%s(%zu letters)", buffer, length);   // appending received string with its length
	size_of_message = strlen(msg);                 // store the length of the stored message
	printk(KERN_INFO "process_list: Received %zu characters from the user\n", length);
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

int my_kernel_module_init(void){
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	
	/*Register with the kernel and indicate that we are registering a character device driver*/
	register_chrdev(majorNumber/*Major Number*/,
	"My procmon"/*Name of the driver*/,
	&my_procmon/*File Operations*/);
	start_iterating();
	return 0;
	/*majorNumber = register_chrdev(164, DEVICE_NAME, &my_procmon);
	if (majorNumber<0){
		printk(KERN_ALERT "EBBChar failed to register a major number\n");
		return majorNumber;
	}
	printk(KERN_INFO "EBBChar: registered correctly with major number %d\n", majorNumber);

	// Register the device class
	ebbcharClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(ebbcharClass)){                // Check for error and clean up if there is
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(ebbcharClass);          // Correct way to return an error on a pointer
	}
	printk(KERN_INFO "EBBChar: device class registered correctly\n");

	// Register the device driver
	ebbcharDevice = device_create(ebbcharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(ebbcharDevice)){               // Clean up if there is an error
		class_destroy(ebbcharClass);           // Repeated code but the alternative is goto statements
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(ebbcharDevice);
	}
	printk(KERN_INFO "EBBChar: device class created correctly\n"); // Made it! device was initialized
	return 0;*/
}

void my_kernel_module_exit(void){
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	/*Unregister the character device driver*/
	unregister_chrdev(majorNumber,"My procmon");
	/*device_destroy(ebbcharClass, MKDEV(majorNumber, 0));     // remove the device
	class_unregister(ebbcharClass);                          // unregister the device class
	class_destroy(ebbcharClass);                             // remove the device class
	unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
	printk(KERN_INFO "process_list: Goodbye from the LKM!\n");*/
}

module_init(my_kernel_module_init);
module_exit(my_kernel_module_exit);
