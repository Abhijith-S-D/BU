#include <unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
int main()
{
    char a=0;
    	printf("pid=%d press y to cause pagefault\n",getpid());
    	scanf("%c",&a);
    long pagesize = sysconf(_SC_PAGESIZE);
    unsigned char *p = malloc(pagesize + 1); /* Cross page boundaries. Page fault may occur depending on your allocator / libc implementation. */
    p[0] = 0;        /* Page fault. */
    p[pagesize] = 1; /* Page fault. */
    return 0;
}
