#Kevin Kell
#Distributed Systems Project 1

CFLAGS = -g -Wall
all: rm dump

rm: rm.c
	gcc rm.c $(CFLAGS) -o rm

dump: dump.c
	gcc dump.c $(CFLAGS) -o dump

clean:
	rm rm dump
