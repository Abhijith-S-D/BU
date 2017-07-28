#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<sys/types.h>
#include <errno.h>
#include<time.h>

typedef struct data_def{
	char message[1000];
	int flag;
}DATA;

int main(){
	int fd,flag=0;
	char pid[1000];
	DATA* rec=(DATA*)malloc(sizeof(DATA));
	
	printf("Please enter pid:");
	scanf("%s",pid);
	printf("pid is %s\n",pid);
	if ( (fd = open("/dev/pagefault", O_RDWR))<0) {
		perror(""); printf("error opening /dev/pagefault\n");
		exit(1);
	}
	
	ssize_t ret = write(fd,pid, sizeof(char)*1000);
	if ( ret < 0) {
		fprintf(stderr, "error writing ret=%ld errno=%d perror: ", ret, errno);
		perror("");
	} else {
		int old=-1;
		char soln[1000];
		while(1){
			memset(rec->message,0,sizeof(char)*1000);
			read(fd, (DATA*)rec,sizeof(DATA));
			if((errno==0)&&(rec->message)&&(old!=rec->flag)){
				if(flag){
				sprintf(soln,"%s",rec->message);
				printf("%s\n",soln);
				old=rec->flag;
				}else{
				old=rec->flag;
				flag=1;
				}
			}
		}
		
	}
	free(rec);
	return 0;
}
