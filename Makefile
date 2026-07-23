CC=gcc

CFLAGS=-Wall -Wextra -g -D_XOPEN_SOURCE=700 -Iinclude

SRCS=$(wildcard src/*.c)
TESTS=$(wildcard tests/*.c)

TARGET=test

all:
	$(CC) $(CFLAGS) $(SRCS) $(TESTS) -o $(TARGET)

clean:
	rm -f test