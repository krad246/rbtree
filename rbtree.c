/*
 * rbtree.c
 *
 *  Created on: Mar 21, 2020
 *      Author: krad2
 */

#include <rbtree.h>

static inline void __rb_set_red(rb_node *rb) {
    if (!rb) return;
    rb->__rb_parent_color |= RB_RED;
}

static inline void __rb_set_black(rb_node *rb) {
    if (!rb) return;
    rb->__rb_parent_color |= RB_BLACK;
}

static inline void __rb_set_color(rb_node *rb, int color) {
    if (!rb) return;
    rb->__rb_parent_color = ((unsigned long) rb_parent(rb)) | color;
}

static inline void __rb_set_parent(rb_node *rb, rb_node *parent) {
    if (!rb) return;
    rb->__rb_parent_color = rb_color(rb) | ((unsigned long) parent);
}

static inline void __rb_set_parent_and_color(rb_node *rb,
                       rb_node *parent, int color) {
    if (!rb) return;
    rb->__rb_parent_color = ((unsigned long) parent) | color;
}

static inline void __rb_insert_basic(rb_node *root, rb_node *node,
                                       int (*cmp)(const void *left, const void *right)) {

    rb_node *cursor = root;
    rb_node *cursor_parent = rb_parent(cursor);

    rb_node *next;
    uint8_t left;
    while (cursor != NULL) {
        if (cmp((void *) node, (void *) cursor) < 0) {
            next = rb_left(cursor);
            left = 1;
        } else {
            next = rb_right(cursor);
            left = 0;
        }

        cursor_parent = cursor;
        cursor = next;
    }

    if (left) {
        rb_left(cursor_parent) = node;
    } else {
        rb_right(cursor_parent) = node;
    }

    rb_left(node) = NULL;
    rb_right(node) = NULL;

    __rb_set_parent_and_color(node, cursor_parent, RB_RED);
}

static inline rb_node *__rb_sibling(const rb_node *node) {

    rb_node *parent, *sibling;
    parent = rb_parent(node);

    if (parent == NULL || node == NULL) sibling = NULL;
    else if (rb_left(parent) == node) sibling = rb_right(sibling);
    else sibling = rb_left(parent);

    return sibling;
}

static inline void __rb_replace_child(rb_node *root, rb_node *old, rb_node *new) {
    if (!root) return;
    if (!old) return;
    if (!new) return;

    if (rb_left(root) == old) rb_left(root) = new;
    else if (rb_right(root) == old) rb_right(root) = new;
}

static inline void __rb_left_rotate(rb_node *root) {

    rb_node *upper_root, *pivot;

    upper_root = rb_parent(root);
    pivot = rb_right(root);

    rb_right(root) = rb_left(pivot);
    __rb_set_parent(rb_right(root), root);

    rb_left(pivot) = root;
    __rb_set_parent(root, pivot);

    __rb_set_parent(pivot, upper_root);
    __rb_replace_child(upper_root, root, pivot);
}

static inline void __rb_right_rotate(rb_node *root) {
    rb_node *upper_root, *pivot;

    upper_root = rb_parent(root);
    pivot = rb_left(root);

    rb_left(root) = rb_right(pivot);
    __rb_set_parent(rb_left(root), root);

    rb_right(pivot) = root;
    __rb_set_parent(root, pivot);

    __rb_set_parent(pivot, upper_root);
    __rb_replace_child(upper_root, root, pivot);
}

static inline void __rb_ll_transform(rb_node *node) {
    rb_node *parent, *grandparent;

    parent = rb_parent(node);
    grandparent = rb_parent(parent);

    __rb_right_rotate(grandparent);

    unsigned int pcolor_old = rb_color(parent);
    __rb_set_color(parent, rb_color(grandparent));
    __rb_set_color(grandparent, pcolor_old);
}

static inline void __rb_lr_transform(rb_node *node) {
    rb_node *parent;
    parent = rb_parent(node);

    __rb_left_rotate(parent);
    __rb_ll_transform(rb_left(node));
}

static inline void __rb_rr_transform(rb_node *node) {
    rb_node *parent, *grandparent;

    parent = rb_parent(node);
    grandparent = rb_parent(parent);

    __rb_left_rotate(grandparent);

    unsigned int pcolor_old = rb_color(parent);
    __rb_set_color(parent, rb_color(grandparent));
    __rb_set_color(grandparent, pcolor_old);
}

static inline void __rb_rl_transform(rb_node *node) {
    rb_node *parent;
    parent = rb_parent(node);

    __rb_right_rotate(parent);
    __rb_rr_transform(rb_right(node));
}

static inline void __rb_ancestor_recolor(const rb_node *node) {

    rb_node *parent, *uncle, *grandparent;

    parent = rb_parent(node);
    uncle = __rb_sibling(parent);
    grandparent = rb_parent(parent);

    __rb_set_black(parent);
    __rb_set_black(uncle);
    __rb_set_red(grandparent);
}

static inline void __rb_rebalance(rb_node *node) {

    rb_node *parent, *uncle, *grandparent;

    for (;;) {

        parent = rb_parent(node);
        uncle = __rb_sibling(parent);
        grandparent = rb_parent(parent);

        if (rb_parent(node) == NULL) {
            __rb_set_black(node);
            return;
        }

        if (rb_is_black(parent)) {
            return;
        }

        // restructure
        if (rb_is_black(uncle) || uncle == NULL) {

            // left-left
            if ((parent == rb_left(grandparent)) && (node == rb_left(parent))) {
                __rb_ll_transform(node);
            }

            // left-right
            else if ((parent == rb_left(grandparent)) && (node == rb_right(parent))) {
                __rb_lr_transform(node);
            }

            // right-right
            else if ((parent == rb_right(grandparent)) && (node == rb_right(parent))) {
                __rb_rr_transform(node);
            }

            // right-left
            else if ((parent == rb_right(grandparent)) && node == (rb_left(parent))) {
                __rb_rl_transform(node);
            }

        // recolor
        } else if (rb_is_red(uncle)) {
            __rb_ancestor_recolor(node);
            node = grandparent;
        }
    }
}

void rb_tree_init(rb_tree *root) {
    root->node = NULL;
    root->count = 0;
}

void rb_node_init(rb_node *node) {
    rb_left(node) = NULL;
    rb_right(node) = NULL;
    RB_CLEAR_NODE(node);
}

void rb_insert(rb_tree *root, rb_node *node,
                                     int (*cmp)(const void *left, const void *right)) {
    if (!root) return;
    if (!node) return;
    if (!cmp) return;

    // if the tree is empty, then set the root of the tree to a known black node
    if (RB_EMPTY_ROOT(root)) {
        root->node = node;
        root->count++;
        __rb_set_parent_and_color(root->node, NULL, RB_BLACK);
        return;
    }

    // insertion with red coloring followed by autobalancing
    __rb_insert_basic(root->node, node, cmp);
    __rb_rebalance(node);

    // retrace the root (has no parent)
    while (rb_parent(node) != NULL) {
        node = rb_parent(node);
    }

    root->node = node;
    root->count++;
}

void rb_insert_lcached(rb_tree_lcached *root, rb_node *node,
                       int (*cmp)(const void *left, const void *right)) {

    if (RB_EMPTY_ROOT(&root->tree)) {
        rb_first_cached(root) = node;
    }

    if (cmp((void *) node, (void *) rb_first_cached(root)) < 0) {
        rb_first_cached(root) = node;
    }

    rb_insert(&root->tree, node, cmp);
}

void rb_insert_rcached(rb_tree_rcached *root, rb_node *node,
                       int (*cmp)(const void *left, const void *right)) {

    if (RB_EMPTY_ROOT(&root->tree)) {
        rb_last_cached(root) = node;
    }

    if (cmp((void *) node, (void *) rb_last_cached(root)) >= 0) {
        rb_last_cached(root) = node;
    }

    rb_insert(&root->tree, node, cmp);
}

void rb_insert_lrcached(rb_tree_lrcached *root, rb_node *node,
                       int (*cmp)(const void *left, const void *right)) {

    if (RB_EMPTY_ROOT(&root->tree)) {
        rb_first_cached(root) = node;
        rb_last_cached(root) = node;
    }

    if (cmp((void *) node, (void *) rb_first_cached(root)) < 0) {
        rb_first_cached(root) = node;
    }

    if (cmp((void *) node, (void *) rb_last_cached(root)) >= 0) {
        rb_last_cached(root) = node;
    }

    rb_insert(&root->tree, node, cmp);
}

const rb_node *rb_find(const rb_tree *root,
                       const void *key, int (*cmp)(const void *left, const void *right)) {
    if (!root) return NULL;
    if (!key) return NULL;
    if (!cmp) return NULL;

    rb_node *cursor = root->node;
    if (cursor == NULL) return NULL;

    int comparison;
    rb_node *next;
    while (cursor != NULL) {

        comparison = cmp((void *) key, (void *) cursor);
        if (comparison < 0) {
            next = cursor->left;
        } else if (comparison == 0) {
            break;
        } else {
            next = cursor->right;
        }

        cursor = next;
    }

    return cursor;
}

static inline const rb_node *__rb_first(const rb_node *anchor) {

    rb_node *cursor = (rb_node *) anchor;
    if (cursor == NULL) return NULL;

    while (rb_left(cursor) != NULL) {
        cursor = rb_left(cursor);
    }

    return cursor;
}

static inline const rb_node *__rb_last(const rb_node *anchor) {

    rb_node *cursor = (rb_node *) anchor;
    if (cursor == NULL) return NULL;

    while (rb_right(cursor) != NULL) {
        cursor = rb_right(cursor);
    }

    return cursor;
}

const rb_node *rb_first(const rb_tree *root) {
    if (!root) return NULL;
    return __rb_first(root->node);
}

const rb_node *rb_last(const rb_tree *root) {
    if (!root) return NULL;
    return __rb_last(root->node);
}

const rb_node *rb_next(const rb_node *node) {
    if (!node) return NULL;
    if (RB_EMPTY_NODE(node)) return NULL;

    if (rb_right(node)) {
        return __rb_first(rb_right(node));
    }

    rb_node *cursor, *cursor_parent;

    cursor = (rb_node *) node;
    for (;;) {
        cursor_parent = rb_parent(cursor);
        if (cursor != rb_right(cursor_parent)) break;
        else cursor = cursor_parent;
    }

    return cursor_parent;
}

const rb_node *rb_prev(const rb_node *node) {
    if (!node) return NULL;
    if (RB_EMPTY_NODE(node)) return NULL;

    if (rb_left(node)) {
        return __rb_last(rb_right(node));
    }

    rb_node *cursor, *cursor_parent;

    cursor = (rb_node *) node;
    for (;;) {
        cursor_parent = rb_parent(cursor);
        if (cursor != rb_left(cursor_parent)) break;
        else cursor = cursor_parent;
    }

    return cursor_parent;
}

static inline void __rb_inorder_foreach(rb_node *anchor, void (*cb)(void *key)) {
    if (!anchor) return;

    __rb_inorder_foreach(anchor->left, cb);
    cb(anchor);
    __rb_inorder_foreach(anchor->right, cb);
}

static inline void __rb_postorder_foreach(rb_node *anchor, void (*cb)(void *key)) {
    if (!anchor) return;

    __rb_postorder_foreach(anchor->left, cb);
    __rb_postorder_foreach(anchor->right, cb);
    cb(anchor);
}

static inline void __rb_preorder_foreach(rb_node *anchor, void (*cb)(void *key)) {
    if (!anchor) return;

    cb(anchor);
    __rb_preorder_foreach(anchor->left, cb);
    __rb_preorder_foreach(anchor->right, cb);
}

void rb_inorder_foreach(rb_tree *tree, void (*cb)(void *key)) {
    __rb_inorder_foreach(rb_root(tree), cb);
}

void rb_postorder_foreach(rb_tree *tree, void (*cb)(void *key)) {
    __rb_postorder_foreach(rb_root(tree), cb);
}

void rb_preorder_foreach(rb_tree *tree, void (*cb)(void *key)) {
    __rb_preorder_foreach(rb_root(tree), cb);
}
