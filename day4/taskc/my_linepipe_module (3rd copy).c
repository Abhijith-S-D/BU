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

#define MAXLEN 100
#define TRUE  1
#define FALSE	0

MODULE_LICENSE("GPL");

static int    majorNumber=164;
static int size_of_message;
static int    numberOpens = 0;
int N=1;
module_param(N,int,0664);

typedef struct data_def{
	char message[MAXLEN];
}DATA;

static DATA** my_queue;
static DATA* msg;
static int front=-1;
static int rear=-1;

int Enqueue(void);
int Dequeue(void);
int isEmpty(void);
int isFull(void);


int Enqueue() {
	if(isFull())
	{
		printk(KERN_ALERT"Error: Queue is Full\n");
		return 0;
	}
	if (isEmpty())
	{ 
		front = rear = 0; 
	}
	else
	{
	    rear = (rear+1)%N;
	}
	my_queue[rear]=(DATA*)kzalloc(sizeof(DATA),GFP_KERNEL);
	my_queue[rear] = msg;
	return 1;
}


int Dequeue() {
	DATA* msg=(DATA*)kzalloc(sizeof(DATA),GFP_KERNEL);
	if(isEmpty())
	{
		printk(KERN_ALERT"Error: Queue is Empty\n");
		return 0;
		
	}
	else if(front == rear ) 
	{
		msg=my_queue[front];
		rear = front = -1;
		
	}
	else
	{
		msg=my_queue[front];
		front = (front+1)%N;
	}
	return 1;
}

int isEmpty() {
    return (front == -1 && rear == -1);
}

int isFull(){
	return ((rear+1)%N == front) ? 1 : 0;
}

static DEFINE_SEMAPHORE(sem1);
static DEFINE_SEMAPHORE(sem2);
static DEFINE_MUTEX(mut);



int linepipe_open(struct inode *pinode, struct file *pfile){
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	numberOpens++;
   	printk(KERN_INFO "process_list: Device has been opened %d time(s)\n", numberOpens);
	return 0;
}

ssize_t linepipe_read(struct file *pfile, char __user *buffer,size_t length, loff_t *offset){
	int error_count = 0,flag;
	down_interruptible(&sem2);
	mutex_lock_interruptible(&mut);
	
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	// copy_to_user has the format ( * to, *from, size) and returns 0 on success
	if(msg){
		kfree(msg);
	}
	msg = (DATA*) kzalloc(sizeof (DATA),GFP_KERNEL);
	flag = Dequeue();
	if(flag!=0){
		if(access_ok(VERIFY_WRITE,buffer,size_of_message)>0){
			error_count = copy_to_user(buffer, msg, sizeof(DATA));
		}else{
			printk(KERN_ALERT"sorry couldnt write to user space");
		}

		if (error_count==0){            // if true then have success
			printk(KERN_INFO"SENT: %s\n",msg->message);
			printk(KERN_INFO "EBBChar: Sent %d characters to the user\n", size_of_message);
			mutex_unlock(&mut);
			up(&sem1);
			return (size_of_message=0);  // clear the position to the start and return 0
		}
		else {
			printk(KERN_INFO "EBBChar: Failed to send %d characters to the user\n", error_count);
			mutex_unlock(&mut);
			up(&sem1);
			return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
		}
	}else{
		mutex_unlock(&mut);
		up(&sem1);
		return -EFAULT;
	}
}

ssize_t linepipe_write(struct file *pfile,const char __user *buffer,size_t length, loff_t *offset){
	int error_count = 0;
	down_interruptible(&sem1);
	mutex_lock_interruptible(&mut);
	
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	if(msg){
		kfree(msg);
	}
	msg = (DATA*) kzalloc(sizeof (DATA),GFP_KERNEL);
	if(access_ok(VERIFY_WRITE,buffer,length)>0){
		error_count = copy_from_user(msg->message,(char*)buffer, length);
		}else{
			printk(KERN_ALERT"sorry couldnt write to kernel space");
		}

	if (error_count==0){            // if true then have success
		size_of_message = strlen(msg->message);
		printk(KERN_INFO "linepipe: Received %s\n",msg->message);
		printk(KERN_INFO "EBBChar: Received %d characters from user\n", size_of_message);
		if(Enqueue())
			printk(KERN_INFO"enqued properly\n");
		mutex_unlock(&mut);
		up(&sem2);
		return size_of_message;  // clear the position to the start and return 0
	}
	else {
		printk(KERN_INFO "EBBChar: Failed to send %d characters to the user\n", error_count);
		mutex_unlock(&mut);
		up(&sem2);
		return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
	}
}

int linepipe_release(struct inode *pinode,struct file *pfile){
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	return 0;
}

struct file_operations my_linepipe = {
	.owner	= THIS_MODULE,
	.open	= linepipe_open,
	.read	= linepipe_read,
	.write	= linepipe_write,
	.release	= linepipe_release,
};

int my_kernel_module_init(void){
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	printk(KERN_ALERT "PARAMETER is %d\n",N);
	/*Register with the kernel and indicate that we are registering a character device driver*/
	register_chrdev(majorNumber/*Major Number*/,
	"My linepipe"/*Name of the driver*/,
	&my_linepipe/*File Operations*/);
	my_queue=(DATA**)kzalloc(N*sizeof(DATA*),GFP_KERNEL);
	sema_init(&sem1, N);
	sema_init(&sem2, 0);
	mutex_init(&mut);
	return 0;
}

void my_kernel_module_exit(void){
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	kfree(my_queue);
	/*Unregister the character device driver*/
	unregister_chrdev(majorNumber,"My linepipe");
}

module_init(my_kernel_module_init);
module_exit(my_kernel_module_exit);
