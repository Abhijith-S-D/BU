#include<stdio.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<unistd.h>

int main(int argc,char* argv[]){
	if(argc!=3){
		printf("Please run in this format ./a.out H C\n");
	}else{
		int h=atoi(argv[1]),i,status,pid;
		int c=atoi(argv[2]);
		printf("(%d): Process starting\n",getpid());
		printf("(%d): Parent's id = (%d)\n",getpid(),getppid());
		printf("(%d): Height in the tree = (%d)\n",getpid(),h);
		if(h>1){
			for(i=0;i<c;i++){
				if((pid=fork())==0){
					char ht[7],ch[7];
					sprintf(ht,"%d",h-1);
					sprintf(ch,"%d",c);
					execlp("./a.out","a.out",ht,ch,(char*)NULL);
				}else{
					while(wait(&status)>0);
				}
			}
			printf("(%d): Terminating at height (%d)\n",getpid(),h);
		}else{
			printf("(%d): Terminating at height (%d)\n",getpid(),h);
		}
	}
	return 0;
}
