#ifndef BACKEND_H_
#define BACKEND_H_

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

//add two integers a and b
int addInts(int a, int b);

//multiply two integers a and b
int multiplyInts(int a, int b);

//divide two float values where b cannot be zero
float divideFloats(float a, float b);

//return the factorial of x
uint16_t factorial(int x);


#endif //BACKEND_H

