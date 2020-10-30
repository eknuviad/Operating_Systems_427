#include "sut.h"
#include <stdio.h>
#include <string.h>
char dest[256] ="0.0.0.0"; 
int port = 3001;


void hello1() {
    int i;
    char *str;
    sut_open(dest, port);
    for (i = 0; i < 100; i++) {
    // printf("reading %d\n", i);
	str = sut_read();
	if (strlen(str) != 0)
	    printf("I am SUT-One, message from server: %s\n", str);
	else
	    printf("ERROR!, empty message received \n");
    // printf("yielding %d\n", i);
	sut_yield();
    }
    sut_close();
    sut_exit();
}

void hello2() {
    int i;
    for (i = 0; i < 100; i++) {
	printf("Hello world!, this is SUT-Two %d\n", i);
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
