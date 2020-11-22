# .DEFAULT_GOAL=all

CC=gcc
CFLAGS=-fsanitize=signed-integer-overflow -fsanitize=undefined -g -std=gnu99 -O2 -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wshadow

SMA=sma.o

sma: sma.c 
	$(CC) -o $(SMA) -c sma.c 
clean:
	rm -rf $(SMA)