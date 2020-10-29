#include "sut.h"
#include <stdio.h>
#include <string.h>
char dest[256] ="0.0.0.0"; 
int port = 3001;

void hello1() {
    int i;
    char sbuf[128];
    char *str;
    // printf("above sut open\n");
    sut_open(dest,port);
    // printf("below sut open\n");
    for (i = 0; i < 20; i++) {
    // printf("writing %d\n", i);
	sprintf(sbuf, "echo Hello world!, message from SUT-One i = %d \n", i);
	sut_write(sbuf, strlen(sbuf));
    // printf("reading %d\n", i);
    char *output = sut_read();
    printf ("Result output: %s\n", output);
    // printf("yielding %d\n", i);
	sut_yield();
    }
    // printf("closing One\n");
    sut_close();
    // printf("exiting One\n");
    sut_exit();
}

void hello2() {
    int i;
    for (i = 0; i < 20; i++) {
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
