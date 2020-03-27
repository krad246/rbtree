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
	rb_node_init(&foo.node);
	```

1. Start working with the tree! A full working snippet can be found [here](example/main.c).

## API

```c
void rb_tree_init(rb_tree *root);
void rb_node_init(rb_node *node);

void rb_insert(rb_tree *root, rb_node *node, int (*cmp)(const void *left, const void *right));
void rb_insert_lcached(rb_tree_lcached *root, rb_node *node, int (*cmp)(const void *left, const void *right));
void rb_insert_rcached(rb_tree_rcached *root, rb_node *node, int (*cmp)(const void *left, const void *right));
void rb_insert_lrcached(rb_tree_lrcached *root, rb_node *node, int (*cmp)(const void *left, const void *right));

void rb_delete(rb_tree *tree, rb_node *node,
               int (*cmp)(const void *left, const void *right), void (*copy)(const void *left, void *right));
void rb_delete_lcached(rb_tree_lcached *tree, rb_node *node,
               int (*cmp)(const void *left, const void *right), void (*copy)(const void *left, void *right));
void rb_delete_rcached(rb_tree_rcached *tree, rb_node *node,
               int (*cmp)(const void *left, const void *right), void (*copy)(const void *left, void *right));
void rb_delete_lrcached(rb_tree_lrcached *tree, rb_node *node,
               int (*cmp)(const void *left, const void *right), void (*copy)(const void *left, void *right));

const rb_node *rb_find(const rb_tree *root, const void *key, int (*cmp)(const void *left, const void *right));

const rb_node *rb_first(const rb_tree *root);
const rb_node *rb_last(const rb_tree *root);

const rb_node *rb_next(const rb_node *node);
const rb_node *rb_prev(const rb_node *node);

void rb_inorder_foreach(rb_tree *tree, void (*cb)(void *key));
void rb_postorder_foreach(rb_tree *tree, void (*cb)(void *key));
void rb_preorder_foreach(rb_tree *tree, void (*cb)(void *key));
```