# Makefile used for ush
# CSCI 347 Spring 2020
#
CC=gcc
CFLAGS=-g -Wall

SRCS = ush.c expand.c builtin.c
OUTF = ${SRCS:.c=.o}

ush: $(OUTF)
	$(CC) $(CFlags) -o ush $(OUTF)

clean:
	rm -f ush $(OUTF)

# dependency list

ush.o expand.o builtin.o: defn.h
