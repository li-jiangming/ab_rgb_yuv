.PHONY: all clean

TARGET=demo

CC=gcc

CFLAGS=-I. \
	   -g3 -std=c11

LDFLAGS=

SRC=$(wildcard *.c)
OBJ=$(SRC:%.c=%.o)

all:$(TARGET)

$(TARGET):$(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(TARGET) $(OBJ)
