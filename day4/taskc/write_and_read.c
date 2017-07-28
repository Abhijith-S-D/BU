#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<sys/types.h>
#include <errno.h>

typedef struct data_def{
	char message[MAXLEN];
}DATA;

int main(){
	int fd;
	char* msg="hello";
	if ( (fd = open("/dev/linepipe", O_RDWR))<0) {
		perror(""); printf("error opening /dev/linepipe\n");
		exit(1);
	}
	/*ssize_t ret = write(fd,msg, 6);
	if ( ret < 0) {
		fprintf(stderr, "error writing ret=%ld errno=%d perror: ", ret, errno);
		perror("");
	} else {
		printf("Bytes written: %ld\n", ret);
	}*/
	DATA* rec=(DATA*)malloc(sizeof(DATA));
	read(fd, rec,sizeof(DATA));
	ssize_t ret=sizeof(rec->message);
	if( ret > 0) {
		printf("Line read: %s\n", rec->message);
		printf("Bytes read: %ld\n", ret);
	} else {
		fprintf(stderr, "error reading ret=%ld errno=%d perror: ", ret, errno);
		perror("");
	}
	return 0;
}
