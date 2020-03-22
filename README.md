# rbtree
A red-black tree implementation in C modeled after the Linux Kernel rbtree.

## Usage

1. Import `rbtree.c` and `rbtree.h`.
2. Embed `rb_node` structures into the objects you wish to link together. For instance, if you wanted to link `struct a` objects together, you would provide the following definition:

	```c
	struct a {
		int x;
		rb_node node;
	}	
	```

3.  Provide a comparator callback for these tree to sort these structures, for instance:

	```c
	int cmp(const void *left, const void *right) {
		struct a *l = rb_entry_safe(left, struct a, node);
		struct a *r = rb_entry_safe(right, struct a, node);

		return l->x - r->x;
	}
	```

	Use `rb_entry` or `rb_entry_safe` to convert the arguments, which are pointers to the nodes embedded in the structure, into the structures of interest:

	```c
	rb_entry_safe(ptr, struct a, node);
	```

	This function takes `ptr` and returns a pointer to its containing structure of type `struct a` given that `ptr` is pointing to `node` within the object.

4.	Create your red-black tree and nodes out of the backing memory you have allocated:

	```c
	rb_tree tree;
	rb_tree_init(&tree);

	struct a foo;
	foo.x = 1;
	rb_node_init(&foo.node);
	```

5. Start working with the tree!

	```c

	// Initializations
	rb_tree tree;
	rb_tree_init(&tree);

	struct a foo;
	foo.x = 1;
	rb_node_init(&foo.node);

	struct a bar;
	foo.x = 2;
	rb_node_init(&bar.node);

	// Insertions
	rb_insert(&tree, &foo.node, cmp);
	rb_insert(&tree, &bar.node, cmp);

	// Removals
	// TODO

	// Iteration
	rb_node *f = rb_first(&tree);
	rb_node *l = rb_last(&tree);

	f = rb_next(f);

	// Searching
	rb_node *p = rb_find(&tree, f, cmp);

	struct a j;
	j.x = -10;
	rb_node_init(&j.y);

	rb_node *q = rb_find(&tree, &j.y, cmp);
	```

## API

```c
void rb_tree_init(rb_tree *root);
void rb_node_init(rb_node *node);

void rb_insert(rb_tree *root, rb_node *node, int (*cmp)(const void *left, const void *right));
void rb_insert_lcached(rb_tree_lcached *root, rb_node *node, int (*cmp)(const void *left, const void *right));
void rb_insert_rcached(rb_tree_rcached *root, rb_node *node, int (*cmp)(const void *left, const void *right));
void rb_insert_lrcached(rb_tree_lrcached *root, rb_node *node, int (*cmp)(const void *left, const void *right));

const rb_node *rb_find(const rb_tree *root, const void *key, int (*cmp)(const void *left, const void *right));

const rb_node *rb_first(const rb_tree *root);
const rb_node *rb_last(const rb_tree *root);

const rb_node *rb_next(const rb_node *node);
const rb_node *rb_prev(const rb_node *node);

void rb_inorder_foreach(rb_tree *tree, void (*cb)(void *key));
void rb_postorder_foreach(rb_tree *tree, void (*cb)(void *key));
void rb_preorder_foreach(rb_tree *tree, void (*cb)(void *key));
```