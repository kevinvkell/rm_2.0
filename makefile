#Kevin Kell
#Distributed Systems Project 1

CFLAGS = -g -Wall
all: rm

rm: rm.c
	gcc rm.c $(CFLAGS) -o rm

clean:
	rm rm
