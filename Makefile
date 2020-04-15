# Makefile used for ush
# CSCI 347 Spring 2020
#
CFLAGS=-g -Wall
ush: ush.o
	$(CC) $(CFlags) -o ush ush.o
clean:
	rm -f ush ush.o
