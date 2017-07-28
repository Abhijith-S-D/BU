#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// this code assumes a maximum line length of 100 bytes.
#define MAXLEN 100

typedef struct data_def{
	char message[MAXLEN];
}DATA;

int main(int argc, char *argv[])
{

	int fd;
	DATA* rec=(DATA*)malloc(sizeof(DATA));
	size_t len = 0;

	if( argc != 2) {
		printf("Usage: %s <linepipe_name>\n", argv[0]);
		exit(1);
	}

	if ( (fd = open(argv[1], O_RDONLY)) < 0) {
		perror(""); printf("error opening %s\n", argv[1]);
		exit(1);
	}

	while(1) {
		// read a line
		
		read(fd, rec,sizeof(DATA));
		ssize_t ret=sizeof(rec->message);
		if( ret > 0) {
			printf("Line read: %s\n", rec->message);
			printf("Bytes read: %ld\n", ret);
		} else {
			fprintf(stderr, "error reading ret=%ld errno=%d perror: ", ret, errno);
			perror("");
			sleep(1);
		}
	}
	free(rec);
	close(fd);

	return 0;
}

