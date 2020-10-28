#include "sut.h"
#include <stdio.h>
#include <string.h>
char dest[256] ="0.0.0.0"; 
int port = 3001;

void hello1() {
    int i;
    char sbuf[128];
    char *str;
    sut_open(dest,port);
    for (i = 0; i < 100; i++) {
	sprintf(sbuf, "Hello world!, message from SUT-One i = %d \n", i);
	sut_write(sbuf, strlen(sbuf));
    str = sut_read();
    if (strlen(str) != 0)
	    printf("%s\n", str);
	else
	    printf("ERROR!, empty message received \n");
	// sut_yield();
    // }
	sut_yield();
    }
    sut_exit();
}

void hello2() {
    int i;
    for (i = 0; i < 100; i++) {
	printf("Hello world!, this is SUT-Two i = %d\n", i);
	sut_yield();
    }
    sut_exit();
}

int main() {
    sut_init();
    sut_create(hello1);
    sut_create(hello2);
    sut_shutdown();
}
