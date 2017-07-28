/*
 * Here's a sample kernel module showing the use of jprobes to dump
 * the arguments of _do_fork().
 *
 * For more information on theory of operation of jprobes, see
 * Documentation/kprobes.txt
 *
 * Build and insert the kernel module as done in the kprobe example.
 * You will see the trace data in /var/log/messages and on the
 * console whenever _do_fork() is invoked to create a new process.
 * (Some messages may be suppressed if syslogd is configured to
 * eliminate duplicate messages.)
 */

#include <linux/kernel.h>
#include <linux/kprobes.h>
#include<linux/time.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/moduleparam.h>
#include<linux/printk.h>
#include<linux/device.h>
#include<asm/uaccess.h>
#include<linux/sched.h>
#include<linux/slab.h>
#include<linux/fs.h> /* For the character driver support */
#include<linux/semaphore.h>
#include<linux/mutex.h>
long int PID=1;
int majorNumber=164;
module_param(PID,long,0664);
char* msg;
int size_of_message;
typedef struct data_def{
	char message[1000];
	int flag;
}DATA;
int count=0;
//static DEFINE_MUTEX(mut);
/*
 * Jumper probe for _do_fork.
 * Mirror principle enables access to arguments of the probed routine
 * from the probe handler.
 */

/* Proxy routine having the same arguments as actual _do_fork() routine */

static long j_handle_mm_fault(struct mm_struct *mm,
			struct vm_area_struct *vma, unsigned long address,
			unsigned int flags)
{
	
	if(PID==mm->owner->pid){
		if(msg)
			kfree(msg);
		msg=(char*)kzalloc(sizeof(char)*1000,GFP_KERNEL);
		struct timespec * t=(struct timespec *)kzalloc(sizeof(struct timespec),GFP_KERNEL);
		getnstimeofday(t);
		sprintf(msg,"%ld,%lld.%.9ld\0\n",address,(long long)t->tv_sec, t->tv_nsec);
		size_of_message = sizeof(msg);
		pr_info("pid=%ld vmstart=%ld vmend=%ld vadd_where_fault_occured=%ld time=%lld.%.9ld\n",mm->owner->pid,vma->vm_start,vma->vm_end,address,(long long)t->tv_sec, t->tv_nsec);
		//mutex_unlock(&mut);
		count++;
		}

	/* Always end with a call to jprobe_return(). */
	jprobe_return();
	return 0;
}

static struct jprobe my_jprobe = {
	.entry			= j_handle_mm_fault,
	.kp = {
		.symbol_name	= "handle_mm_fault",
	},
};

int pagefault_open(struct inode *pinode, struct file *pfile){
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	return 0;
}

ssize_t pagefault_read(struct file *pfile, char __user *buffer,size_t length, loff_t *offset){
	int error_count = 0;
	DATA* d=(DATA*) kzalloc(sizeof (DATA),GFP_KERNEL);
	strncpy(d->message,msg,100*sizeof(char));
	d->flag=count;
	//printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	// copy_to_user has the format ( * to, *from, size) and returns 0 on success
	
	if(access_ok(VERIFY_WRITE,buffer,sizeof(DATA))>0){
		//mutex_lock_interruptible(&mut);
		//printk("%ld\n",msg->address);
		error_count = copy_to_user(buffer, d, sizeof(DATA));
		kfree(d);
	}else{
		printk(KERN_ALERT"sorry couldnt write to user space");
	}

	if (error_count==0){            // if true then have success
		//printk(KERN_INFO"SENT: details\n");
		return (size_of_message=0);  // clear the position to the start and return 0
	}
	else {
		printk(KERN_INFO "pagefault: Failed to send details\n");
		return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
	}
	
	
}

ssize_t pagefault_write(struct file *pfile,const char __user *buffer,size_t length, loff_t *offset){
	int error_count = 0;
	char pid1[1000];
	char* pid=(char*)kzalloc(sizeof(char)*1000,GFP_KERNEL);
	char *temp=(char*)kzalloc(sizeof(char),GFP_KERNEL);
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	if(access_ok(VERIFY_WRITE,buffer,length)>0){
		error_count = copy_from_user(pid1,(char*)buffer, length);
		sprintf(pid,"%s",pid1);
		}else{
			printk(KERN_ALERT"sorry couldnt write to kernel space");
		}

	if (error_count==0){ 
		int i=0;
		while(pid[i]!='\0'){
			i++;
		}        // if true then have success
		printk("i=%d\n",i);
		printk("%s\n",pid);
		
		temp=pid+i;
		PID=simple_strtol(pid,&temp,10);
		printk("%ld\n",PID);
		printk(KERN_INFO "pagefault: monitoring pid: %ld\n",PID);
		return sizeof(msg);
	}
	else {
		printk(KERN_INFO "EBBChar: Failed to receive pid from the user,currently monitoring pid:%ld\n", PID);
		return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
	}
}

int pagefault_release(struct inode *pinode,struct file *pfile){
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	return 0;
}
struct file_operations my_pagefault = {
	.owner	= THIS_MODULE,
	.open	= pagefault_open,
	.read	= pagefault_read,
	.write	= pagefault_write,
	.release	= pagefault_release,
};

static int __init page_fault_init(void)
{
	int ret;

	ret = register_jprobe(&my_jprobe);
	if (ret < 0) {
		printk(KERN_INFO "register_jprobe failed, returned %d\n", ret);
		return -1;
	}
	printk(KERN_INFO "Planted jprobe at %p, handler addr %p\n",
	       my_jprobe.kp.addr, my_jprobe.entry);
	/*Register with the kernel and indicate that we are registering a character device driver*/
	register_chrdev(majorNumber/*Major Number*/,
	"My pagefault"/*Name of the driver*/,
	&my_pagefault/*File Operations*/);
	//mutex_init(&mut);
	//mutex_lock_interruptible(&mut);
	msg=(char*)kzalloc(sizeof(char)*1000,GFP_KERNEL);
	
	return 0;
}

static void __exit page_fault_exit(void)
{
	unregister_jprobe(&my_jprobe);
	unregister_chrdev(majorNumber,"My pagefault");
	//mutex_unlock(&mut);
	printk(KERN_INFO "jprobe at %p unregistered\n", my_jprobe.kp.addr);
}


module_init(page_fault_init)
module_exit(page_fault_exit)
MODULE_LICENSE("GPL");
