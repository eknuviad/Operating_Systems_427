# .DEFAULT_GOAL=all

CC=gcc
CFLAGS=-fsanitize=signed-integer-overflow -fsanitize=undefined -g -std=gnu99 -O2 -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wshadow

SUT=sut.o

sut: sut.c 
	$(CC) -o $(SUT) -c sut.c 
clean:
	rm -rf $(SUT)

