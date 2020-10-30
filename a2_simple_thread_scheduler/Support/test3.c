#include "sut.h"
#include <stdio.h>
char dest[256] ="0.0.0.0"; 
int port = 3001;

void hello0() {
    int i;
    // sut_open(dest,port);
    for (i = 0; i < 30; i++) {
	printf("Hello world!, this is SUT-Zero %d\n", i);
	sut_yield();
    }
    sut_exit();
}

void hello1() {
    int i;
    // sut_open(dest,port);
    for (i = 0; i < 30; i++) {
	printf("Hello world!, this is SUT-One %d\n", i);
	sut_yield();
    }
    sut_exit();
}

void hello2() {
    int i;
    // sut_open(dest,port);
    for (i = 0; i < 30; i++) {
	printf("Hello world!, this is SUT-Two %d\n", i);
	sut_yield();
    }
    sut_exit();
}

void hello3() {
    int i;
    // sut_open(dest,port);
    for (i = 0; i < 30; i++) {
	printf("Hello world!, this is SUT-Three %d\n", i);
	sut_yield();
	sut_create(hello0);
    }
    sut_exit();
}

int main() {
    sut_init();
    sut_create(hello1);
    sut_create(hello2);
    sut_create(hello3);
    sut_shutdown();
}
