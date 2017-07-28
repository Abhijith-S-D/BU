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

typedef struct node_def{
	DATA* data;
	struct node_def* next; 
}NODE;

typedef struct Queue {
    NODE *head;
    NODE *tail;
    int size;
    int limit;
} Queue;

 Queue* my_queue;
 NODE* temp;
 DATA* msg;
 
void ConstructQueue(int limit);
void DestructQueue(void);
int Enqueue( NODE *item);
NODE *Dequeue(void);
int isEmpty(void);

void ConstructQueue(int limit) {
    my_queue = (Queue*) kzalloc(sizeof(Queue),GFP_KERNEL);
    if (my_queue == NULL) {
        
    }else{
    if (limit <= 0) {
        limit = 100;
    }
    my_queue->limit = limit;
    my_queue->size = 0;
    my_queue->head = NULL;
    my_queue->tail = NULL;
    }
}

void DestructQueue(void) {
    NODE *pN;
    while (!isEmpty()) {
        pN = Dequeue();
        kfree(pN);
    }
    kfree(my_queue);
}

int Enqueue( NODE *item) {
	printk(KERN_INFO"ENQUEING %s\n",item->data->message);
    /* Bad parameter */
    if ((my_queue == NULL) || (item == NULL)) {
        return FALSE;
    }
    // if(my_queue->limit != 0)
    if (my_queue->size >= my_queue->limit) {
        return FALSE;
    }
    /*the queue is empty*/
    item->next = NULL;
    if (my_queue->size == 0) {
        my_queue->head = item;
        my_queue->tail = item;
        printk(KERN_INFO"FIRST ENQUEING %s\n",my_queue->head->data->message);

    } else {
        /*adding item to the end of the queue*/
        my_queue->tail->next = item;
        my_queue->tail = item;
        printk(KERN_INFO"ADDED %s\n",my_queue->tail->data->message);
    }
    my_queue->size++;
    return TRUE;
}


NODE * Dequeue() {
    /*the queue is empty or bad param*/
    NODE *item=(NODE*) kzalloc(sizeof (NODE),GFP_KERNEL);
    if (isEmpty())
        return NULL;
    item = my_queue->head;
    my_queue->head = (my_queue->head)->next;
    my_queue->size--;
    printk(KERN_INFO"DEQUEUING %s\n",item->data->message);
    return item;
}

int isEmpty() {
    if (my_queue == NULL) {
        return FALSE;
    }
    if (my_queue->size == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
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
	int error_count = 0;
	down_interruptible(&sem2);
	mutex_lock_interruptible(&mut);
	
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	// copy_to_user has the format ( * to, *from, size) and returns 0 on success
	if(temp){
		kfree(temp);
	}
	temp = (NODE*) kzalloc(sizeof (NODE),GFP_KERNEL);
	temp = Dequeue();
	if(temp!=NULL){
		if(access_ok(VERIFY_WRITE,buffer,size_of_message)>0){
			error_count = copy_to_user(buffer, temp->data, sizeof(DATA));
		}else{
			printk(KERN_ALERT"sorry couldnt write to user space");
		}

		if (error_count==0){            // if true then have success
			printk(KERN_INFO"SENT: %s\n",temp->data->message);
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
	if(temp){
		kfree(temp);
	}
	temp = (NODE*) kzalloc(sizeof (NODE),GFP_KERNEL);
	temp->data=(DATA*) kzalloc(sizeof (DATA),GFP_KERNEL);
	if(msg){
		kfree(msg);
	}
	msg=(DATA*)kzalloc(sizeof (DATA),GFP_KERNEL);
	if(access_ok(VERIFY_WRITE,buffer,length)>0){
		error_count = copy_from_user(msg->message,(char*)buffer, length);
		}else{
			printk(KERN_ALERT"sorry couldnt write to kernel space");
		}

	if (error_count==0){            // if true then have success
		temp->data=msg;
		size_of_message = strlen(temp->data->message);
		printk(KERN_INFO "linepipe: Received %s\n",temp->data->message);
		printk(KERN_INFO "EBBChar: Received %d characters from user\n", size_of_message);
		if(Enqueue(temp))
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
	ConstructQueue(N);
	sema_init(&sem1, N);
	sema_init(&sem2, 0);
	mutex_init(&mut);
	return 0;
}

void my_kernel_module_exit(void){
	printk(KERN_ALERT "Inside the %s function\n", __FUNCTION__);
	DestructQueue();
	/*Unregister the character device driver*/
	unregister_chrdev(majorNumber,"My linepipe");
}

module_init(my_kernel_module_init);
module_exit(my_kernel_module_exit);
