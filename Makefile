CC = gcc
CFLAGS = -Wall -O3 -DRB_UNSAFE=0
CXX = g++

OBJS := rbtree.o rbtree_lcached.o rbtree_rcached.o rbtree_lrcached.o

-include $(OBJS:.o=.d)

%.o: %.c
	gcc -c $(CFLAGS) $*.c -o $*.o
	gcc -MM $(CFLAGS) $*.c > $*.d

rblib.a: $(OBJS)
	ar rcs rblib.a $(OBJS)

all: rblib.a
	
.PHONY: clean
clean:
	rm -f *.o *.d *.a *.gch *.exe
