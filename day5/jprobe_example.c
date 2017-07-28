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
#include <linux/module.h>
#include <linux/kprobes.h>
#include<linux/time.h>
#include<linux/slab.h>
long PID=1;
module_param(PID,long,0664);
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
		struct timespec *time=(struct timespec *)kmalloc(sizeof(struct timespec),GFP_KERNEL);
		getnstimeofday(time);
		pr_info("pid=%ld vmstart=%ld vmend=%ld vadd_where_fault_occured=%ld time=%lld.%.9ld\n",mm->owner->pid,vma->vm_start,vma->vm_end,address,(long long)time->tv_sec, time->tv_nsec);
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

static int __init jprobe_init(void)
{
	int ret;

	ret = register_jprobe(&my_jprobe);
	if (ret < 0) {
		printk(KERN_INFO "register_jprobe failed, returned %d\n", ret);
		return -1;
	}
	printk(KERN_INFO "Planted jprobe at %p, handler addr %p\n",
	       my_jprobe.kp.addr, my_jprobe.entry);
	return 0;
}

static void __exit jprobe_exit(void)
{
	unregister_jprobe(&my_jprobe);
	printk(KERN_INFO "jprobe at %p unregistered\n", my_jprobe.kp.addr);
}

module_init(jprobe_init)
module_exit(jprobe_exit)
MODULE_LICENSE("GPL");
