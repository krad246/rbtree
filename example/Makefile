CC = gcc
CFLAGS = -Wall
CXX = g++

rbtest: main.o rbtree.o
	$(CC) main.o rbtree.o -o $@

main.o: main.c rbtree.h
	$(CC) $(CFLAGS) -g main.c -c -o $@

rbtree.o: rbtree.c rbtree.h
	$(CC) $(CFLAGS) -g rbtree.c -c -o $@

.PHONY: clean
clean:
	-rm *.o rbtest