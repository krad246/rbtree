CC = gcc

rbtest: main.o rbtree.o
	$(CC) main.o rbtree.o -o $@

main.o: main.c rbtree.h
	$(CC) -g main.c -c -o $@

rbtree.o: rbtree.c rbtree.h
	$(CC) -g rbtree.c -c -o $@

.PHONY: clean
clean:
	-rm *.o rbtest