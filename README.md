# rbtree
A red-black tree implementation in C modeled after the Linux Kernel rbtree.

## Usage

1. Import `rbtree.c` and `rbtree.h`.
2. Embed `rb_node` structures into the objects you wish to link together. For instance, if you wanted to link `struct a` objects together, you would provide the following definition:

	```c
	struct a {
		int x;
		rb_node node;
	};	
	```

	Use `rb_entry` or `rb_entry_safe` to convert the arguments, which are pointers to the nodes embedded in the structure, into the structures of interest:

	```c
	rb_entry_safe(ptr, struct a, node);
	```

	This function takes `ptr` and returns a pointer to its containing structure of type `struct a` given that `ptr` is pointing to `node` within the object.

3.  Provide a comparator and copy callback for these tree to sort these structures, for instance:

	```c
	int cmp(const void *left, const void *right) {
		struct a *l = rb_entry_safe(left, struct a, node);
		struct a *r = rb_entry_safe(right, struct a, node);

		return l->x - r->x;
	}

	void copy(const void *src, void *dst) {
		struct a *l = rb_entry_safe(src, struct a, node);
		struct a *r = rb_entry_safe(dst, struct a, node);

		r->x = l->x;
	}
	```

4.	Create your red-black tree and nodes out of the backing memory you have allocated:

	```c
	rb_tree tree;
	rb_tree_init(&tree);

	struct a foo;
	foo.x = 1;

	rb_tree_insert(&foo.node);
	```