/*
 * rbtree.h
 *
 *  Created on: Mar 17, 2020
 *      Author: krad2
 */

#ifndef RBTREE_H_
#define RBTREE_H_

#include <stddef.h>
#include <stdint.h>

typedef struct __attribute__((aligned(sizeof(long)))) rb_node_t {
    unsigned long  __rb_parent_color;
    struct rb_node_t *left;
    struct rb_node_t *right;
} rb_node;

typedef struct rb_tree_t {
    rb_node *node;
    size_t count;
} rb_tree;

typedef struct rb_tree_lcached {
    rb_tree tree;
    rb_node *leftmost;
} rb_tree_lcached;

typedef struct rb_tree_rcached {
    rb_tree tree;
    rb_node *rightmost;
} rb_tree_rcached;

typedef struct rb_tree_lrcached {
    rb_tree tree;

    rb_node *leftmost;
    rb_node *rightmost;
} rb_tree_lrcached;

#define RB_RED      0
#define RB_BLACK    1

#define __rb_color(pc)     ((pc) & 1)
#define __rb_is_black(pc)  __rb_color(pc)
#define __rb_is_red(pc)    (!__rb_color(pc))
#define rb_color(rb)       __rb_color((rb)->__rb_parent_color)
#define rb_is_red(rb)      __rb_is_red((rb)->__rb_parent_color)
#define rb_is_black(rb)    __rb_is_black((rb)->__rb_parent_color)

#define __rb_parent(pc)    ((struct rb_node_t *)(pc & ~3))
#define rb_parent(r)   __rb_parent((r)->__rb_parent_color)

#define rb_left(rb)         ((rb)->left)
#define rb_right(rb)        ((rb)->right)

#define rb_root(tree)        ((tree)->node)
#define RB_EMPTY_ROOT(root)  (rb_root(root) == NULL)

/* 'empty' nodes are nodes that are known not to be inserted in an rbtree */
#define RB_EMPTY_NODE(node)  \
    ((node)->__rb_parent_color == (unsigned long)(node))
#define RB_CLEAR_NODE(node)  \
    ((node)->__rb_parent_color = (unsigned long)(node))

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define rb_entry(ptr, type, member) container_of(ptr, type, member)
#define rb_entry_safe(ptr, type, member) \
    ({ typeof(ptr) ____ptr = (ptr); \
       ____ptr ? rb_entry(____ptr, type, member) : NULL; \
    })

#define rb_first_cached(root)   (root)->leftmost
#define rb_last_cached(root)    (root)->rightmost

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

#endif /* RBTREE_H_ */
