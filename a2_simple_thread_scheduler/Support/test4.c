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
    for (i = 0; i < 5; i++) {
    // printf("one i is %d\n", i);
	sprintf(sbuf, "echo Hello world!, message from SUT-One i = %d \n", i);
	sut_write(sbuf, strlen(sbuf));
    char *output = sut_read();
    printf ("Result output: %s\n", output);
	sut_yield();
    }
    sut_exit();
}

void hello2() {
    int i;
    for (i = 0; i < 5; i++) {
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
