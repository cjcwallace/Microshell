# Cameron Wallace
# April 14, 2020
# CSCI 347 Spring 2020
# Assignment 2
# Makefile used for ush

CC=gcc
CFLAGS=-g -Wall

SRCS = ush.c expand.c builtin.c strmode.c
OUTF = ${SRCS:.c=.o}

ush: $(OUTF)
	$(CC) $(CFlags) -o ush $(OUTF)

clean:
	rm -f ush $(OUTF) *.c# core

# dependency list

ush.o expand.o builtin.o: defn.h
