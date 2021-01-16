# Makefile for ex2-q1 winter 2020A
CFLAGS = -g -Wall
LDFLAGS = -lm # not really needed for this exercise

#CC = gcc -std=c99
CC = gcc -pthread
ECHO = echo "going to compile for target $@"
PROG1 = ex4_q1
PROG2 = item_reporter
PROGS = $(PROG1) $(PROG2)

all: $(PROG1) $(PROG2)
	./$(PROG1)
	./$(PROG2)

$(PROG1): ex4_q1.o ex4_q1_given.o
	$(CC) $(CFLAGS) ex4_q1_given.o ex4_q1.o -o $(PROG1) $(LDFLAGS)

#test: $(PROGS)
#	./$(PROG1) > out.log 2> err.log

item_reporter.o:  item_reporter.h item_reporter.c 
	$(CC) $(CFLAGS) -c item_reporter.c $(LDFLAGS)

ex4_q1.o:  ex4_q1.h ex4_q1.c
	$(CC) $(CFLAGS) -c ex4_q1.c $(LDFLAGS)

ex4_q1_given.o:  ex4_q1_given.h ex4_q1_given.c
	$(CC) $(CFLAGS) -c ex4_q1_given.c $(LDFLAGS)

clean: 
	$(RM) *.o *~ $(PROGS) *.tmp *.log
