CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11

OBJS = schedule.o parser.o process.o

schedule: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o schedule

schedule.o: schedule.c process.h parser.h
	$(CC) $(CFLAGS) -c schedule.c

parser.o: parser.c parser.h process.h
	$(CC) $(CFLAGS) -c parser.c

process.o: process.c process.h
	$(CC) $(CFLAGS) -c process.c

demo: demo.c
	$(CC) $(CFLAGS) demo.c -o demo

test: test.c
	$(CC) $(CFLAGS) test.c -o test

all: schedule demo test

clean:
	rm -f *.o schedule demo test