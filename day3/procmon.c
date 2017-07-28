#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>

struct mydata{
	int pid;
	int ppid;
	int cpu;
	long int state;
	int valid;
};

int main(){
	struct mydata* buff=(struct mydata*)malloc(sizeof(struct mydata));
	struct mydata* data=(struct mydata*)malloc(sizeof(struct mydata));
	data->valid=1;
	int fd=open("/dev/process_list",O_RDONLY);
	if(fd==-1)
		perror("open error\n");
	while(data->valid){
		int sz=read(fd,buff,sizeof(char)*131072);
		data=(struct mydata*)buff;
		long int st=data->state;
		if(st==NULL){
			printf("hi thats all :)\n");
		}else{
			printf("PID=%d PPID=%d CPU=%d STATE=",data->pid,data->ppid,data->cpu);
			if(st&0)
				printf("TASK_RUNNING, ");
			if(st&1)
				printf("TASK_INTERRUPTIBLE, ");
			if(st&2)
				printf("TASK_UNINTERRUPTIBLE, ");
			if(st&4)
				printf("__TASK_STOPPED, ");
			if(st&8)
				printf("__TASK_TRACED, ");
			if(st&64)
				printf("TASK_DEAD, ");
			if(st&128)
				printf("TASK_WAKEKILL, ");
			if(st&256)
				printf("TASK_WAKING, ");
			if(st&512)
				printf("TASK_PARKED, ");
			if(st&1024)
				printf("TASK_NOLOAD, ");
			if(st&2048)
				printf("TASK_NEW, ");
			if(st&4096)
				printf("TASK_STATE_MAX, ");
			printf("\n");
		}
	}
	close(fd);
	return 0;
}
