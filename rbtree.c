/*
 * rbtree.c
 *
 *  Created on: Mar 21, 2020
 *      Author: krad2
 */

#include "rbtree.h"

/**
 *	Helper functions for __rb_parent_color member of rb_node.
 */

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

/**
 *	Basic BST insertion algorithm - no balancing yet!
 */

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

/**
 *	Fetches the other child of the given node's parent, if it exists.
 */

static inline rb_node *__rb_sibling(const rb_node *node) {

    rb_node *parent, *sibling;
    parent = rb_parent(node);

    if (parent == NULL || node == NULL) sibling = NULL;
    else if (rb_left(parent) == node) sibling = rb_right(parent);
    else sibling = rb_left(parent);

    return sibling;
}

/**
 *	Links 'new' in place of 'old' on the side of 'root' that 'old' was on.
 */

static inline void __rb_replace_child(rb_node *root, rb_node *old, rb_node *nw) {
    if (!root) return;
    if (!old) return;

    if (rb_left(root) == old) rb_left(root) = nw;
    else if (rb_right(root) == old) rb_right(root) = nw;
}

/**
 * Swaps colors between 2 nodes.
 */

static inline void __rb_swap_colors(rb_node *src, rb_node *dst) {
    unsigned int scolor_old = rb_color(src);        // back up src color
    __rb_set_color(src, rb_color(dst));             // change src to dst color
    __rb_set_color(dst, scolor_old);                // change dst to old src color
}

/*
 *	Tree rotation operations centered on 'root'.
 */

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

/**
 *	Red-black tree ancestor transformations centered on 'node' for insertions.
 */

static inline void __rb_ins_ll_transform(rb_node *node, rb_node *parent, rb_node *grandparent) {
//    rb_node *parent, *grandparent;
//
//    parent = rb_parent(node);
//    grandparent = rb_parent(parent);

    __rb_right_rotate(grandparent);
    __rb_swap_colors(parent, grandparent);
}

static inline void __rb_ins_lr_transform(rb_node *node, rb_node *parent) {
//    rb_node *parent;
//    parent = rb_parent(node);

    __rb_left_rotate(parent);
    __rb_ins_ll_transform(rb_left(node), node, parent);
}

static inline void __rb_ins_rr_transform(rb_node *node, rb_node *parent, rb_node *grandparent) {
//    rb_node *parent, *grandparent;
//
//    parent = rb_parent(node);
//    grandparent = rb_parent(parent);

    __rb_left_rotate(grandparent);
    __rb_swap_colors(parent, grandparent);

}

static inline void __rb_ins_rl_transform(rb_node *node, rb_node *parent) {
//    rb_node *parent;
//    parent = rb_parent(node);

    __rb_right_rotate(parent);
    __rb_ins_rr_transform(rb_right(node), node, parent);
}

/**
 * Red-black tree recoloring transformation centered on 'node'.
 */

static inline void __rb_ancestor_recolor(const rb_node *node) {

    rb_node *parent, *uncle, *grandparent;

    parent = rb_parent(node);
    uncle = __rb_sibling(parent);
    grandparent = rb_parent(parent);

    __rb_set_black(parent);
    __rb_set_black(uncle);
    __rb_set_red(grandparent);
}

/**
 * Red-black tree rebalancing centered on 'node'. Called after insertion.
 */

static inline void __rb_insert_rebalance(rb_node *node) {

    rb_node *parent, *uncle, *grandparent;

    for (;;) {

        parent = rb_parent(node);
        uncle = __rb_sibling(parent);
        grandparent = rb_parent(parent);

		// root
        if (parent == NULL) {
            __rb_set_black(node);
            return;
        }

		// about to hit the root
        if (rb_is_black(parent)) {
            return;
        }

        // restructuring
        if (rb_is_black(uncle) || uncle == NULL) {

            // left-left
            if ((parent == rb_left(grandparent)) && (node == rb_left(parent))) {
                __rb_ins_ll_transform(node, parent, grandparent);
            }

            // left-right
            else if ((parent == rb_left(grandparent)) && (node == rb_right(parent))) {
                __rb_ins_lr_transform(node, parent);
            }

            // right-right
            else if ((parent == rb_right(grandparent)) && (node == rb_right(parent))) {
                __rb_ins_rr_transform(node, parent, grandparent);
            }

            // right-left
            else if ((parent == rb_right(grandparent)) && node == (rb_left(parent))) {
                __rb_ins_rl_transform(node, parent);
            }

        // recolor
        } else if (rb_is_red(uncle)) {
            __rb_ancestor_recolor(node);
            node = grandparent;
        }
    }
}

/**
 *	Sets tree structures to default state.
 */

void rb_tree_init(rb_tree *root) {
    if (!root) return;

    root->node = NULL;
}

void rb_node_init(rb_node *node) {
    if (!node) return;

    rb_left(node) = NULL;
    rb_right(node) = NULL;
    RB_CLEAR_NODE(node);
}

static inline void __rb_node_clear(rb_node *node) {
    rb_node_init(node);
}

/**
 *	Red-black insertion given a comparator.
 */

void rb_insert(rb_tree *root, rb_node *node,
                                     int (*cmp)(const void *left, const void *right)) {
    if (!root) return;
    if (!node) return;
    if (!cmp) return;

    // if the tree is empty, then set the root of the tree to a known black node
    if (RB_EMPTY_ROOT(root)) {
        root->node = node;
        __rb_set_parent_and_color(root->node, NULL, RB_BLACK);
        return;
    }

    // insertion with red coloring followed by autobalancing
    __rb_insert_basic(root->node, node, cmp);
    __rb_insert_rebalance(node);

    // retrace the root (has no parent)
    while (rb_parent(node) != NULL) {
        node = rb_parent(node);
    }

    root->node = node;
}

/**
 *	Insertion into 'cached' trees is the same as above, but maintains a running max / min.
 */

void rb_insert_lcached(rb_tree_lcached *root, rb_node *node,
                       int (*cmp)(const void *left, const void *right)) {

    // the very first node inserted is the minimum, for now
    if (RB_EMPTY_ROOT(&root->tree)) {
        rb_first_cached(root) = node;
    }

    // subsequent inserts are cached as the min if they're smaller than the known min
    if (cmp((void *) node, (void *) rb_first_cached(root)) < 0) {
        rb_first_cached(root) = node;
    }

    rb_insert(&root->tree, node, cmp);
}

void rb_insert_rcached(rb_tree_rcached *root, rb_node *node,
                       int (*cmp)(const void *left, const void *right)) {

    // the very first node inserted is the minimum, for now
    if (RB_EMPTY_ROOT(&root->tree)) {
        rb_last_cached(root) = node;
    }

    // subsequent inserts are cached as the max if they're larger than the known max
    if (cmp((void *) node, (void *) rb_last_cached(root)) >= 0) {
        rb_last_cached(root) = node;
    }

    rb_insert(&root->tree, node, cmp);
}

void rb_insert_lrcached(rb_tree_lrcached *root, rb_node *node,
                       int (*cmp)(const void *left, const void *right)) {

    // first element inserted is both the largest and smallest
    if (RB_EMPTY_ROOT(&root->tree)) {
        rb_first_cached(root) = node;
        rb_last_cached(root) = node;
    }

    // subsequent inserts are cached as the min if they're smaller than the known min
    if (cmp((void *) node, (void *) rb_first_cached(root)) < 0) {
        rb_first_cached(root) = node;
    }

    // subsequent inserts are cached as the max if they're larger than the known max
    if (cmp((void *) node, (void *) rb_last_cached(root)) >= 0) {
        rb_last_cached(root) = node;
    }

    rb_insert(&root->tree, node, cmp);
}

/**
 *	Binary search to find 'key'. Returns NULL if not found.
 */

static inline const rb_node *__rb_find(const rb_node *anchor,
                                       const void *key, int (*cmp)(const void *left, const void *right)) {

    rb_node *cursor = (rb_node *) anchor;
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

const rb_node *rb_find(const rb_tree *root,
                       const void *key, int (*cmp)(const void *left, const void *right)) {
    return __rb_find(rb_root(root), key, cmp);
}

/**
 *	Traversal to the logical min / max of the tree.
 */

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

/**
 *	Iterative tree traversal in sorted / backwards order.
 */

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
        return __rb_last(rb_left(node));
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

static inline const rb_node *__rb_node_successor(const rb_node *target) {
    rb_node *successor;

    if (rb_left(target) == NULL && rb_right(target) == NULL) {
        successor = NULL;
    } else if (rb_left(target) != NULL && rb_right(target) != NULL) {
        successor = (rb_node *) rb_prev(target);
    } else {
        if (rb_left(target) != NULL) successor = rb_left(target);
        else successor = rb_right(target);
    }

    return successor;
}

static inline void __rb_delete_node(rb_node *target) {
    __rb_replace_child(rb_parent(target), target, NULL);  // disconnect the successor
    __rb_node_clear(target);
}

static inline void __rb_move_and_delete(rb_node *src, rb_node *dst,
                                        void (*copy)(const void *src, void *dst)) {
    copy((void *) src, (void *) dst);
    __rb_delete_node(src);
}

/**
 * Basic BST deletion algorithm - no balancing yet!
 */

static inline void __rb_delete_basic(rb_node *root, rb_node *node,
                                       int (*cmp)(const void *left, const void *right),
                                       void (*copy)(const void *src, void *dst)) {
    rb_node *target, *successor;

    target = (rb_node *) __rb_find(root, node, cmp);                // try to find the node to even delete, if it exists
    if (!target) return;

    successor = (rb_node *) __rb_node_successor(target);            // find the successor
    __rb_move_and_delete(successor, target, copy);                  // copy the successor to the target, then delete the successor
}

static inline void __rb_delete_rebalance(rb_node *node) {

    rb_node *parent, *sibling;
    rb_node *slchild, *srchild;

    for (;;) {

//        // deleting a leaf doesn't require any work
//        if (node == NULL) {
//            return;
//        }

        parent = rb_parent(node);
        sibling = __rb_sibling(node);

        // root
        if (parent == NULL) {
            return;
        }

        // deleting red doesn't break anything
        if (rb_is_red(node)) {
            return;
        }

        // deleting black node is illegal
        if (node == rb_left(parent)) {
            if (rb_is_red(sibling)) {                               // sibling is red, parent is black which is illegal
                __rb_swap_colors(parent, sibling);
                __rb_left_rotate(parent);
                parent = rb_parent(node);
                sibling = __rb_sibling(node);
            }

            slchild = rb_left(sibling);
            srchild = rb_right(sibling);
            if (rb_is_black(slchild) && rb_is_black(srchild)) {     // double-black overflow
                __rb_set_red(sibling);
                node = parent;
            } else {
                if (rb_is_black(srchild)) {
                    __rb_set_black(slchild);
                    __rb_set_red(sibling);
                    __rb_right_rotate(sibling);
                    parent = rb_parent(node);
                    sibling = __rb_sibling(node);
                }
                __rb_set_color(sibling, rb_color(parent));
                __rb_set_black(parent);
                __rb_set_black(srchild);
                __rb_left_rotate(parent);
                return;
            }
        } else {
            if (rb_is_red(sibling)) {
                __rb_swap_colors(parent, sibling);
                __rb_right_rotate(parent);
                parent = rb_parent(node);
                sibling = __rb_sibling(node);
            }

            slchild = rb_left(sibling);
            srchild = rb_right(sibling);

            if (rb_is_black(slchild) && rb_is_black(srchild)) {
                __rb_set_red(sibling);
                node = parent;
            } else {
                if (rb_is_black(slchild)) {
                    __rb_set_black(srchild);
                    __rb_set_red(sibling);
                    __rb_left_rotate(sibling);
                    parent = rb_parent(node);
                    sibling = __rb_sibling(node);
                }
                __rb_set_color(sibling, rb_color(parent));
                __rb_set_black(parent);
                __rb_set_black(slchild);
                __rb_right_rotate(parent);
                return;
            }
        }
    }
}

/**
 * Deletes a node from the tree. Requires a comparator to find the element and a copy() function to exchange elements.
 */

void rb_delete(rb_tree *tree, rb_node *node,
               int (*cmp)(const void *left, const void *right),
               void (*copy)(const void *src, void *dst)) {

    if (!tree) return;
    if (!node) return;
    if (!cmp) return;
    if (!copy) return;

    rb_node *target, *successor;

    target = (rb_node *) __rb_find(rb_root(tree), node, cmp);       // try to find the node to even delete, if it exists
    if (!target) return;

    successor = (rb_node *) __rb_node_successor(target);            // find the successor
    __rb_delete_rebalance(successor);                               // do any necessary rebalancing centered around the successor before deleting
    __rb_move_and_delete(successor, target, copy);                  // copy the successor to the target, then delete the successor

    if (RB_EMPTY_NODE(rb_root(tree))) {                             // deleting the root empties the tree
        rb_root(tree) = NULL;
    }

    // retrace the root (has no parent)
    while (rb_parent(target) != NULL) {
        target = rb_parent(target);
    }

    rb_root(tree) = target;
}

void rb_delete_lcached(rb_tree_lcached *root, rb_node *node,
                       int (*cmp)(const void *left, const void *right),
                       void (*copy)(const void *src, void *dst)) {

    // deleting the min makes the new min the next element in sorted order
    if (cmp((void *) node, (void *) rb_first_cached(root)) == 0) {
            rb_first_cached(root) = (rb_node *) rb_next(rb_first_cached(root));
    }

    rb_delete(&root->tree, node, cmp, copy);

    // if the tree was emptied, we don't have a min
    if (RB_EMPTY_ROOT(&root->tree)) {
        rb_first_cached(root) = NULL;
    }
}

void rb_delete_rcached(rb_tree_rcached *root, rb_node *node,
                       int (*cmp)(const void *left, const void *right),
                       void (*copy)(const void *src, void *dst)) {

    // deleting the max makes the new max the previous element in sorted order
    if (cmp((void *) node, (void *) rb_last_cached(root)) == 0) {
            rb_last_cached(root) = (rb_node *) rb_prev(rb_last_cached(root));
    }

    rb_delete(&root->tree, node, cmp, copy);

    // if the tree was emptied, we don't have a max
    if (RB_EMPTY_ROOT(&root->tree)) {
        rb_last_cached(root) = NULL;
    }
}

void rb_delete_lrcached(rb_tree_lrcached *root, rb_node *node,
                       int (*cmp)(const void *left, const void *right),
                       void (*copy)(const void *src, void *dst)) {

    // deleting the min makes the new min the next element in sorted order
    if (cmp((void *) node, (void *) rb_first_cached(root)) == 0) {
        rb_first_cached(root) = (rb_node *) rb_first(&root->tree);
    }

    // deleting the max makes the new max the previous element in sorted order
    if (cmp((void *) node, (void *) rb_last_cached(root)) == 0) {
            rb_last_cached(root) = (rb_node *) rb_last(&root->tree);
    }

    rb_delete(&root->tree, node, cmp, copy);

    // deleting the whole tree deletes the max and min
    if (RB_EMPTY_ROOT(&root->tree)) {
        rb_first_cached(root) = NULL;
        rb_last_cached(root) = NULL;
    }
}

/**
 * Tree traversal in all 3 'styles'. Invokes 'cb' on each node.
 */

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
