#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#include "rbtree.h"

struct a {
    int x;
    rb_node node;
};

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

int main(int argc, char **argv) {

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
	rb_delete(&tree, &foo.node, cmp, copy);
	rb_delete(&tree, &bar.node, cmp, copy);

	// Iteration
	rb_node *f = (rb_node *) rb_first(&tree);
	rb_node *l = (rb_node *) rb_last(&tree);

	f = (rb_node *) rb_next(f);

	// Searching
	rb_node *p = (rb_node *) rb_find(&tree, f, cmp);

	struct a j;
	j.x = -10;
	rb_node_init(&j.node);

	rb_node *q = (rb_node *) rb_find(&tree, &j.node, cmp);

	return 0;
}