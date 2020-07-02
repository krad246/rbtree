/**
 * @file rbtree.c
 * @author krad2
 * @date June 28 20200
 * @brief A red-black implementation in C, defining an intrusive node-based red-black tree class library.
 * @see https://en.wikipedia.org/wiki/Red%E2%80%93black_tree
 */

#include "rbtree.h"

/**
 * @defgroup rb_member_setters Helper functions for __rb_parent_color member of rb_node_t.
 * @{
 */

#define RB_BLACK 											((uintptr_t) rb_black)
#define RB_RED 												((uintptr_t) rb_red)

/* --- */

static inline void __rb_set_red(rb_node_t *rb) {
	RB_NULL_CHECK(rb);

    rb->__rb_parent_color &= ~RB_BLACK;
	rb->__rb_parent_color |= RB_RED;
}

static inline void __rb_set_black(rb_node_t *rb) {
    RB_NULL_CHECK(rb);

    rb->__rb_parent_color |= RB_BLACK;
}

static inline void __rb_set_color(rb_node_t *rb, rb_color_t color) {
    RB_NULL_CHECK(rb);

	if (color == RB_BLACK) __rb_set_black(rb);
	else __rb_set_red(rb);
}

static inline void __rb_swap_colors(rb_node_t *src, rb_node_t *dst) {
	RB_NULL_CHECK(src);
	RB_NULL_CHECK(dst);

    rb_color_t src_color = rb_color(src);       	/** back up src color */
    __rb_set_color(src, rb_color(dst));             /** change src to dst color */
    __rb_set_color(dst, src_color);                	/** change dst to old src color */
}

static inline void __rb_set_parent(rb_node_t *rb, rb_node_t *parent) {

	/* Concatenates the color bit with the parent pointer by representing the parent pointer as an int. */
	if (rb) rb->__rb_parent_color = rb_color(rb) | ((uintptr_t) parent);
}

static inline void __rb_replace_child(rb_node_t *root, rb_node_t *old, rb_node_t *nw) {
    RB_NULL_CHECK(root);
    RB_NULL_CHECK(old);

	if (root) {

		/* Links 'new' in place of 'old' on the side of 'root' that 'old' was on. */
		if (rb_left(root) == old) rb_left(root) = nw;
    	else if (rb_right(root) == old) rb_right(root) = nw;
	}

	__rb_set_parent(nw, root);
}

static inline void __rb_set_parent_and_color(rb_node_t *rb, rb_node_t *parent, rb_color_t color) {
   	RB_NULL_CHECK(rb);

    __rb_set_parent(rb, parent);
	__rb_set_color(rb, color);
}

/** @} */

/**
 * @defgroup rb_rotations Red-black tree rotation operations
 * @{
 */

static inline void __rb_left_rotate(rb_node_t *root) {
    rb_node_t *upper_root, *pivot;

    upper_root = rb_parent(root); 					/** 'master tree' containing the subtree being rotated */
    pivot = rb_right(root);							/** new root of that subtree */

    rb_right(root) = rb_left(pivot);				/** link the current root's right subtree as the new root's left */
    __rb_set_parent(rb_right(root), root);

    rb_left(pivot) = root;							/** link the old root as the left subtree of the new root */
    __rb_set_parent(rb_left(pivot), pivot);

    __rb_set_parent(pivot, upper_root);				/** update the subtree's connection to the master */
    __rb_replace_child(upper_root, root, pivot);
}

static inline void __rb_right_rotate(rb_node_t *root) {
    rb_node_t *upper_root, *pivot;

    upper_root = rb_parent(root);					/** 'master tree' containing the subtree being rotated */
    pivot = rb_left(root);							/** new root of that subtree */

    rb_left(root) = rb_right(pivot);				/** link the current root's right subtree as the new root's left */
    __rb_set_parent(rb_left(root), root);

    rb_right(pivot) = root;							/** link the old root as the left subtree of the new root */
    __rb_set_parent(rb_right(pivot), pivot);

    __rb_set_parent(pivot, upper_root);				/** update the subtree's connection to the master */
    __rb_replace_child(upper_root, root, pivot);
}

/** @} */

/**
 * @defgroup rb_init Initializes tree structures.
 * @{
 */

/**
 * @brief Makes the node loop back on itself and shed its subtrees.
 */
static inline void __rb_node_clear(rb_node_t *node) {
	RB_NULL_CHECK(node);
	rb_disconnect(node);
}

static void __rb_node_init(rb_node_t *node) {
	__rb_node_clear(node);
}

/**
 * @fn rb_tree_init
 * @details Initializes a rb_tree_t with a NULL root.
 * @param[in] tree Pointer to an rbtree.
 */
void rb_tree_init(rb_tree_t *tree) {
    RB_NULL_CHECK(tree);
	
    rb_root(tree) = NULL;
}

/**
 * @fn rb_tree_lcached_init
 * @details Initializes the tree, then sets the min to NULL.
 * @param[in] tree Pointer to a min-cached rbtree.
 */
void rb_tree_lcached_init(rb_tree_lcached_t *tree) {
	RB_NULL_CHECK(tree);
	rb_tree_init((rb_tree_t *) tree);
	rb_min(tree) = NULL;
}

/**
 * @fn rb_tree_rcached_init
 * @details Initializes the tree, then sets the max to NULL.
 * @param[in] tree Pointer to a max-cached rbtree.
 */
void rb_tree_rcached_init(rb_tree_rcached_t *tree) {
	RB_NULL_CHECK(tree);
	rb_tree_init((rb_tree_t *) tree);
	rb_max(tree) = NULL;
}

/**
 * @fn rb_tree_lrcached_init
 * @details Combination of rb_tree_lcached_init and rb_tree_rcached_init
 * @param[in] tree Pointer to a lrcached_rbtree.
 */
void rb_tree_lrcached_init(rb_tree_lrcached_t *tree) {
	RB_NULL_CHECK(tree);
	rb_tree_init((rb_tree_t *) tree);
	rb_min(tree) = NULL;
	rb_max(tree) = NULL;
}

/** @} */

/**
 * @defgroup rb_insertion Red-black tree insertion functions (and helpers)
 * @{
 */

/**
 *	@brief Fetches the other child of the given node's parent, if it exists.
 */
static inline rb_node_t *__rb_sibling(const rb_node_t *node) {
	RB_NULL_CHECK(node, NULL);
    
	rb_node_t *parent, *sibling;

	/* there's no sibling if we're the root! */
    parent = node ? rb_parent(node) : NULL;
	if (parent == NULL) return NULL;

	/* get the opposite pointer from what the parent sees you as */
    if (rb_left(parent) == node) sibling = rb_right(parent);
    else sibling = rb_left(parent);

    return sibling;
}

/**
 * @brief Performs a standard BST insert.
 * @param[in] root Root of the subtree to perform the insertion on
 * @param[in] node Pointer to the item being inserted.
 * @param[in] cmp Comparator callback used for traversal.
 */
static inline void __rb_insert_basic(rb_node_t *root, rb_node_t *node, int (*cmp)(const rb_node_t *left, const rb_node_t *right)) {
	RB_NULL_CHECK(root);
	RB_NULL_CHECK(node);
	RB_NULL_CHECK(cmp);

    rb_iterator_t cursor = root;
    rb_iterator_t cursor_parent = rb_parent(cursor);

    rb_iterator_t next = NULL;
    bool left = false;

	/* iterative traversal */
    while (cursor) {

		/* less than 0 means go left, otherwise go right */
        if (cmp((const rb_node_t *) node, (const rb_node_t *) cursor) < 0) {
            next = rb_left(cursor);
            left = true;
        } else {
            next = rb_right(cursor);
            left = false;
        }

        cursor_parent = cursor;
        cursor = next;
    }

	/* check which side we stopped on and set the leaf up there */
	__rb_set_parent_and_color((rb_node_t *) node, (rb_node_t *) cursor_parent, RB_RED);
    if (left) {
        rb_left(cursor_parent) = node;
    } else {
        rb_right(cursor_parent) = node;
    }

    rb_left(node) = NULL;
    rb_right(node) = NULL;
}

/**
 * @brief Performs rb_insert_fixup on node, correcting all subtrees above it.
 */
static inline void __rb_insert_rebalance(rb_node_t *node) {
    rb_node_t *parent, *uncle, *grandparent;

    for (;;) {
        parent = node ? rb_parent(node) : NULL;
        uncle = __rb_sibling(parent);
        grandparent = parent ? rb_parent(parent) : NULL;

		/* hitting the root means we're done - make sure it's black afterwards */
        if (parent == NULL) {
			__rb_set_black(node);
            return;
        }

		/* if the node we've hit is black then our the invariant is re-satisfied */
		if (rb_is_black(node)) {
			return;
		}

		/* same thing as above, but needed for 0 - 1 nodes to prevent a segfault. */
		if (rb_is_black(parent)) {
			return;
		}

		/* try a recolor first */
		if (rb_is_red(uncle)) {
            __rb_set_black(parent);
			__rb_set_black(uncle);
			__rb_set_red(grandparent);
            node = grandparent;
			continue;
        } 

		/* left-left */
		if ((parent == rb_left(grandparent)) && (node == rb_left(parent))) {
			__rb_swap_colors(parent, grandparent);
   			__rb_right_rotate(grandparent);
		}

		/* left-right */
		else if ((parent == rb_left(grandparent)) && (node == rb_right(parent))) {
			rb_node_t *center, *center_parent, *center_grandparent;

			/* convert it to the left-left case */
			__rb_left_rotate(parent); 

			/**
			 * Our frame of reference has changed, so reestablish it for the LL transform. 
			 * Left rotating the parent will pull it down to the same level as the node, 
			 * so the new parent and grandparent for the LL transform are different.
			 */
			center = parent;	
			center_parent = rb_parent(center);
			center_grandparent = rb_parent(center_parent);

			__rb_swap_colors(center_parent, center_grandparent);
   			__rb_right_rotate(center_grandparent);
		}

		/* right-right */
		else if ((parent == rb_right(grandparent)) && (node == rb_right(parent))) {
			__rb_swap_colors(parent, grandparent);
    		__rb_left_rotate(grandparent);
		}

		/* right-left */
		else if ((parent == rb_right(grandparent)) && node == (rb_left(parent))) {
			rb_node_t *center, *center_parent, *center_grandparent;

			/* convert it to the right-right case */
			__rb_right_rotate(parent);

			/**
			 * Our frame of reference has changed, so reestablish it for the RR transform. 
			 * Right rotating the parent will pull it down to the same level as the node, 
			 * so the new parent and grandparent for the RR transform are different.
			 */
			center = parent;
			center_parent = rb_parent(center);
			center_grandparent = rb_parent(center_parent);

			__rb_swap_colors(center_parent, center_grandparent);
    		__rb_left_rotate(center_grandparent);
		}

		/* move to the next level after rebalancing the current one */
		node = parent;
    }
}

/**
 * @fn rb_tree_insert_at
 * @brief Inserts a node into an rb_tree as close as possible to and after the provided iterator.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Pointer to an rb_node instance embedded in something else.
 * @param[in] hint Pointer to a valid rb_node used as a starting position for the insertion.
 * @param[in] cmp Comparator callback used to traverse the tree.
 */
void rb_tree_insert_at(rb_tree_t *tree, rb_node_t *node, rb_iterator_t hint, int (*cmp)(const rb_node_t *left, const rb_node_t *right)) {
	RB_NULL_CHECK(tree);
	RB_NULL_CHECK(node);
	RB_NULL_CHECK(hint);
	RB_NULL_CHECK(cmp);

	/**
	 * check that the node can fit in this window, i.e. the hint is valid.
	 * first, fetch the other edge of the window where the node should be slotted.
	 */
	const rb_iterator_t next_pos = rb_next(hint);

	/* first check the left edge and see if the node can be placed after the hint */
	int hint_left_cmp = cmp((const rb_node_t *) hint, (const rb_node_t *) node);

	/** 
	 * then check the right edge, and see if it suggests that the node is appropriately placed.
	 * a NULL right edge is fine, that just means we are appending to the right edge of the tree.
	 */
	int hint_right_cmp = next_pos ? cmp((const rb_node_t *) next_pos, (const rb_node_t *) node) : 0;

	/* we default to a standard root-anchored insert if the hint is bad. */
	bool hint_is_less = (hint_left_cmp < 0);
	bool hint_successor_is_more = (hint_right_cmp >= 0);
	bool hint_is_valid = hint_is_less && hint_successor_is_more;
	if (hint_is_valid) {

		/* the node needs to be fresh and must not come in corrupted */
		__rb_node_init(node);

		/* insert it starting from the hint */
		__rb_insert_basic(hint, node, cmp);
		__rb_insert_rebalance(node);

		/** 
		 * 'node' (and possibly the whole tree have since been nodified 
		 * so we trace 'node' back up to find the new root, and then save that 
		 */
		while (rb_parent(node) != NULL) {
			node = rb_parent(node);
		}

		rb_root(tree) = node;
	} else rb_tree_insert(tree, node, cmp);
}

/**
 * @fn rb_tree_lcached_insert_at
 * @brief Inserts a node into an rb_tree as close as possible to and after the provided iterator. Updates the min.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Pointer to an rb_node instance embedded in something else.
 * @param[in] hint Pointer to a valid rb_node used as a starting position for the insertion.
 * @param[in] cmp Comparator callback used to traverse the tree.
 */
void rb_tree_lcached_insert_at(rb_tree_lcached_t *tree, rb_node_t *node, rb_iterator_t hint, int (*cmp)(const rb_node_t *left, const rb_node_t *right)) {
	RB_NULL_CHECK(tree);
	RB_NULL_CHECK(rb_root(tree));
	RB_NULL_CHECK(node);
	RB_NULL_CHECK(hint);
	RB_NULL_CHECK(cmp);

	/* subsequent inserts may be smaller, so we update the min accordingly */ 
    if (cmp((const rb_node_t *) node, (const rb_node_t *) rb_min(tree)) <= 0) rb_min(tree) = node;

	rb_tree_insert_at(tree, node, hint, cmp);
}

/**
 * @fn rb_tree_rcached_insert_at
 * @brief Inserts a node into an rb_tree as close as possible to and after the provided iterator. Updates the max.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Pointer to an rb_node instance embedded in something else.
 * @param[in] hint Pointer to a valid rb_node used as a starting position for the insertion.
 * @param[in] cmp Comparator callback used to traverse the tree.
 */
void rb_tree_rcached_insert_at(rb_tree_rcached_t *tree, rb_node_t *node, rb_iterator_t hint, int (*cmp)(const rb_node_t *left, const rb_node_t *right)) {
	RB_NULL_CHECK(tree);
	RB_NULL_CHECK(rb_root(tree));
	RB_NULL_CHECK(node);
	RB_NULL_CHECK(hint);
	RB_NULL_CHECK(cmp);

	/* subsequent inserts may be bigger, so update the max accordingly */
	if (cmp((const rb_node_t *) node, (const rb_node_t *) rb_max(tree)) >= 0) rb_max(tree) = node;

	rb_tree_insert_at(tree, node, hint, cmp);
}

/**
 * @fn rb_tree_lrcached_insert_at
 * @brief Inserts a node into an rb_tree as close as possible to and after the provided iterator. Updates the min and the max.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Pointer to an rb_node instance embedded in something else.
 * @param[in] hint Pointer to a valid rb_node used as a starting position for the insertion.
 * @param[in] cmp Comparator callback used to traverse the tree.
 */
void rb_tree_lrcached_insert_at(rb_tree_lrcached_t *tree, rb_node_t *node, rb_iterator_t hint, int (*cmp)(const rb_node_t *left, const rb_node_t *right)) {	
	RB_NULL_CHECK(tree);
	RB_NULL_CHECK(rb_root(tree));
	RB_NULL_CHECK(node);
	RB_NULL_CHECK(hint);
	RB_NULL_CHECK(cmp);

	/* subsequent inserts may be smaller, so we update the min accordingly */ 
    if (cmp((const rb_node_t *) node, (const rb_node_t *) rb_min(tree)) <= 0) rb_min(tree) = node;

	/* subsequent inserts may be bigger, so update the max accordingly */
	if (cmp((const rb_node_t *) node, (const rb_node_t *) rb_max(tree)) >= 0) rb_max(tree) = node;

	rb_tree_insert_at(tree, node, hint, cmp);
} 

/**
 * @fn rb_tree_insert
 * @brief Inserts a node into an rb_tree, guided by a comparator.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Pointer to an rb_node instance embedded in something else.
 * @param[in] cmp Comparator callback used to traverse the tree.
 */
void rb_tree_insert(rb_tree_t *tree, rb_node_t *node, int (*cmp)(const rb_node_t *left, const rb_node_t *right)) {
	RB_NULL_CHECK(tree);
	RB_NULL_CHECK(node);
	RB_NULL_CHECK(cmp);

	/* base case, tree is empty so we just initialize the root node to this one */
    if (rb_is_empty(tree)) {
        rb_root(tree) = node;
        __rb_set_parent_and_color(rb_root(tree), NULL, RB_BLACK);

	/* the 'hint' is the root because the minimum distance to the root arrangement is a balanced tree */
    } else {

		/* the node needs to be fresh and must not come in corrupted */
		__rb_node_init(node);
		__rb_insert_basic(rb_root(tree), node, cmp);
		__rb_insert_rebalance(node);

		/** 
		 * 'node' (and possibly the whole tree have since been nodified 
		 * so we trace 'node' back up to find the new root, and then save that 
		 */
		while (rb_parent(node) != NULL) {
			node = rb_parent(node);
		}

		rb_root(tree) = node;
	}
}

/**
 * @fn rb_tree_lcached_insert
 * @brief Inserts a node into an rb_tree, guided by a comparator. Maintains and updates a running min.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Pointer to an rb_node instance embedded in something else.
 * @param[in] cmp Comparator callback used to traverse the tree.
 */
void rb_tree_lcached_insert(rb_tree_lcached_t *tree, rb_node_t *node, int (*cmp)(const rb_node_t *left, const rb_node_t *right)) {
	RB_NULL_CHECK(tree);
	RB_NULL_CHECK(node);
	RB_NULL_CHECK(cmp);

	/* base case of no nodes means that the first is also the min */
    if (rb_is_empty(tree)) rb_min(tree) = node; 

	/* subsequent inserts may be smaller, so we update the min accordingly */ 
    if (cmp((const rb_node_t *) node, (const rb_node_t *) rb_min(tree)) <= 0) rb_min(tree) = node;

	/* then we do a standard insert */
    rb_tree_insert((rb_tree_t *) tree, node, cmp);
}

/**
 * @fn rb_tree_rcached_insert
 * @brief Inserts a node into an rb_tree, guided by a comparator. Maintains and updates a running max.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Pointer to an rb_node instance embedded in something else.
 * @param[in] cmp Comparator callback used to traverse the tree.
 */
void rb_tree_rcached_insert(rb_tree_rcached_t *tree, rb_node_t *node, int (*cmp)(const rb_node_t *left, const rb_node_t *right)) {
	RB_NULL_CHECK(tree);
	RB_NULL_CHECK(node);
	RB_NULL_CHECK(cmp);
	
	/* base case of no nodes means that the first is also the max */
	if (rb_is_empty(tree)) rb_max(tree) = node;

	/* subsequent inserts may be bigger, so update the max accordingly */
	if (cmp((const rb_node_t *) node, (const rb_node_t *) rb_max(tree)) >= 0) rb_max(tree) = node;

	/* then we do a standard insert */
    rb_tree_insert((rb_tree_t *) tree, node, cmp);
}

/**
 * @fn rb_tree_lrcached_insert
 * @brief Combination of rb_tree_rcached_insert and rb_tree_lcached_insert.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Pointer to an rb_node instance embedded in something else.
 * @param[in] cmp Comparator callback used to traverse the tree.
 */
void rb_tree_lrcached_insert(rb_tree_lrcached_t *tree, rb_node_t *node, int (*cmp)(const rb_node_t *left, const rb_node_t *right)) {
	RB_NULL_CHECK(tree);
	RB_NULL_CHECK(node);
	RB_NULL_CHECK(cmp);

	/* base case of no nodes means that the first is the min and the max */
	if (rb_is_empty(tree)) {
		rb_min(tree) = node;
		rb_max(tree) = node;
	}

	/* subsequent inserts may change these cached values, so update as needed */
	if (cmp((const rb_node_t *) node, (const rb_node_t *) rb_min(tree)) <= 0) rb_min(tree) = node;
	if (cmp((const rb_node_t *) node, (const rb_node_t *) rb_max(tree)) >= 0) rb_max(tree) = node;

	rb_tree_insert(tree, node, cmp);
}

/** @} */

/**
 * @defgroup rb_deletion Red-black tree deletion functions (and helpers)
 * @{
 */

/**
 * @brief Finds the first element greater than the target node.
 */
static inline const rb_node_t *__rb_node_successor(const rb_node_t *target) {
	RB_NULL_CHECK(target, NULL);
	rb_node_t *successor;

	/* if the target is not a leaf node, the one right after is the successor */
	if (rb_left(target) && rb_right(target)) {
		successor = (rb_node_t *) rb_next((rb_iterator_t) target);

	/* if there is 1 child, that's always the successor */
	} else if (!rb_right(target)) {
		successor = rb_left(target);
	} else if (!rb_left(target)) {
		successor = rb_right(target);

	/* no children means there's no replacement for you and you just get deleted */
	} else {
		successor = NULL;
	}

    return successor;
}

/**
 * @brief Finds the first element greater than the target node.
 */
static inline const rb_node_t *__rb_node_predecessor(const rb_node_t *target) {
	RB_NULL_CHECK(target, NULL);
	rb_node_t *successor;

	/* if the target is not a leaf node, the one right before is the successor */
	if (rb_left(target) && rb_right(target)) {
		successor = (rb_node_t *) rb_prev((rb_iterator_t) target);

	/* if there is 1 child, that's always the successor */
	} else if (!rb_right(target)) {
		successor = rb_left(target);
	} else if (!rb_left(target)) {
		successor = rb_right(target);

	/* no children means there's no replacement for you and you just get deleted */
	} else {
		successor = NULL;
	}

    return successor;
}

/**
 * @brief Deletes a node by connecting its subtrees to its parent.
 */
static inline void __rb_delete_node(rb_node_t *target) {
	RB_NULL_CHECK(target);

	rb_node_t *child;

	if (rb_left(target)) child = rb_left(target);
	else if (rb_right(target)) child = rb_right(target);
	else child = NULL;

    __rb_replace_child(rb_parent(target), target, child);
	__rb_set_parent(child, rb_parent(target));
    __rb_node_clear(target);
}

/**
 * @brief Basic copy-and-remove algorithm used by BST basic delete.
 * @param[in] src Deletion target, i.e. copy source node.
 * @param[in] dst Node that gets replaced.
 * @param[in] copy Copy callback for the deletion.
 */
static inline void __rb_move_and_delete(rb_node_t *src, rb_node_t *dst, void (*copy)(const rb_node_t *src, rb_node_t *dst)) {

	/* if we had 1 child (so a replacement) then we copy the replacement and delete that */
	if (src) {
		copy((void *) src, (void *) dst);
    	__rb_delete_node(src);

	/* else we just delete ourselves */
	} else {
		__rb_delete_node(dst);
	}
}

static inline void __rb_delete_basic(rb_node_t *replacement, rb_node_t *target, void (*copy)(const rb_node_t *src, rb_node_t *dst)) {
	__rb_move_and_delete(replacement, target, copy);
}

/**
 * @brief Performs rb_delete_fixup on the subtree centered on node, correcting all surrounding trees.
 */
static inline void __rb_delete_rebalance(rb_node_t *node) {
    rb_node_t *parent, *sibling;
    rb_node_t *sibling_lchild, *sibling_rchild;

    for (;;) {
        parent = rb_parent(node);

		/* if we hit the root, we're done, and the root must always be black. */
        if (parent == NULL) {
			__rb_set_black(node);
            break;
        }

		/* deleting a red doesn't break any invariants. */
		if (rb_is_red(node)) {
			__rb_set_black(node);
			break;
		}

        /* otherwise black height property is in violation and we'll need the sibling, etc. */
        sibling = __rb_sibling(node);

        /* if we have a red nearby, let's try to dump the double black into that nearby red... 
		 * that'll prep us for the next case
		 */
        if (rb_is_red(sibling)) {
			__rb_set_black(sibling); 									/* migrate the red upwards */
			__rb_set_red(parent);

			if (sibling == rb_right(parent)) __rb_left_rotate(parent);  /* rotation will make a black w/ red children tree. */
			else __rb_right_rotate(parent);

			sibling = __rb_sibling(node);								/* our frame of reference has now changed. */
        }

        /* new sibling is ALWAYS black, so we have to dump the double black somewhere else */
        sibling_lchild = sibling ? rb_left(sibling) : NULL;
        sibling_rchild = sibling ? rb_right(sibling) : NULL;

        /* if the nephew / niece can't take the black recolor, try to propagate it up and dissolve it further up */
        if (rb_is_black(sibling_lchild) && rb_is_black(sibling_rchild)) {
	    	__rb_set_red(sibling);
			node = parent;
			continue;
        }

        /* otherwise, we can do some rotations to find a red to drop the double-black into. */
        if (sibling == rb_right(parent)) {

            /* if the only red available is on the left side of the sibling, pull the red up and adjust black height */
            if (rb_is_black(sibling_rchild)) {

				/* right-left case right here */
				__rb_set_black(sibling_lchild);
				__rb_set_red(sibling);
                __rb_right_rotate(sibling);	
     
				sibling = __rb_sibling(node);
				sibling_lchild = sibling ? rb_left(sibling) : NULL;
        		sibling_rchild = sibling ? rb_right(sibling) : NULL;
            }

            /* right-right case */
			__rb_set_color(sibling, rb_color(parent));
            __rb_set_black(parent);
            __rb_set_black(sibling_rchild);
            __rb_left_rotate(parent);
            break;

        } else {

            /* left-right case */
            if (rb_is_black(sibling_lchild)) {
				__rb_set_black(sibling_rchild);
				__rb_set_red(sibling);
                __rb_left_rotate(sibling);

				sibling = __rb_sibling(node);
				sibling_lchild = sibling ? rb_left(sibling) : NULL;
        		sibling_rchild = sibling ? rb_right(sibling) : NULL;
            }

			/* left-left case */
            __rb_set_color(sibling, rb_color(parent));
            __rb_set_black(parent);
            __rb_set_black(sibling_lchild);
            __rb_right_rotate(parent);
            break;
        }
	}
}

/**
 * @fn rb_tree_delete_at
 * @brief Deletes a node from a rbtree at an iterator.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[in] cmp Comparator callback used to traverse the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 * @return Iterator to the next element.
 */
rb_iterator_t rb_tree_delete_at(rb_tree_t *tree, rb_iterator_t node, void (*copy)(const rb_node_t *src, rb_node_t *dst)) {
	RB_NULL_CHECK(tree, NULL);
	RB_NULL_CHECK(node, NULL);
	RB_NULL_CHECK(copy, NULL);

	rb_node_t *replacement, *cursor;

	/* we don't need to find the node, only its replacement */
    replacement = (rb_node_t *) __rb_node_predecessor(node);    

	/* then we rebalance and propagate changes from the base up */
	if (replacement) __rb_delete_rebalance(replacement);	
	else __rb_delete_rebalance(node);
	
	/* retrace the root because the rotations might've changed it */
    cursor = node;
	while (rb_parent(cursor) != NULL) {
        cursor = rb_parent(cursor);
    }

	/* now that all the references in the tree are correctly updated and valid, we can delete this node problem-free */
	rb_iterator_t next_node = rb_next(node);
	__rb_delete_basic(replacement, node, copy);

	/* if the root is cleared, then the tree doesn't exist anymore */
    if (rb_is_disconnected(rb_root(tree))) {                             
        rb_root(tree) = NULL;
    } else {
		rb_root(tree) = cursor;
	}

	return next_node;
}

/**
 * @fn rb_tree_lcached_delete_at
 * @brief Deletes a node from a rbtree at an iterator. Updates the min.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[in] cmp Comparator callback used to traverse the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 * @return Iterator to the next element.
 */
rb_iterator_t rb_tree_lcached_delete_at(rb_tree_lcached_t *tree, rb_node_t *node, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst)) {

	/* check if the min of the tree changed. if it did, slide the min pointer forward */
    bool min_changed = false;
    if (cmp((const rb_node_t *) node, (const rb_node_t *) rb_min(tree)) == 0) min_changed = true;
	if (min_changed) rb_min(tree) = rb_next(node);

	/* delete, update references, do whatever you need to do */
    rb_iterator_t next_node = rb_tree_delete_at((rb_tree_t *) tree, node, copy);
    
	/* then update the min */
	if (rb_is_empty(tree)) {
		rb_min(tree) = NULL;
    }

	return next_node;
}

/**
 * @fn rb_tree_rcached_delete_at
 * @brief Deletes a node from a rbtree at an iterator. Updates the max.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[in] cmp Comparator callback used to traverse the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 * @return Iterator to the next element.
 */
rb_iterator_t rb_tree_rcached_delete_at(rb_tree_rcached_t *tree, rb_node_t *node, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst)) {

	/* check if the max of the tree changed. if it did, slide the max pointer forward */
    bool max_changed = false;
    if (cmp((const rb_node_t *) node, (const rb_node_t *) rb_max(tree)) == 0) max_changed = true;
	if (max_changed) rb_max(tree) = rb_prev(rb_max(tree));

	/* delete, update references, do whatever you need to do */
    rb_iterator_t next_node = rb_tree_delete_at((rb_tree_t *) tree, node, copy);

	/* then update the max */
    if (rb_is_empty(tree)) {
		rb_max(tree) = NULL;
    }

	return next_node;
}

/**
 * @fn rb_tree_lrcached_delete_at
 * @brief Deletes a node from a rbtree at an iterator. Updates the min and the max.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[in] cmp Comparator callback used to traverse the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 * @return Iterator to the next element.
 */
rb_iterator_t rb_tree_lrcached_delete_at(rb_tree_lrcached_t *tree, rb_iterator_t node, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst)) {

	/* check if the min of the tree changed. if if it did, slide the min pointer forward */
    bool min_changed = false;
    if (cmp((const rb_node_t *) node, (const rb_node_t *) rb_min(tree)) == 0) min_changed = true;
	if (min_changed) rb_min(tree) = rb_next(node);

	/* check if the max of the tree changed. if it did, slide the max pointer forward */
    bool max_changed = false;
    if (cmp((const rb_node_t *) node, (const rb_node_t *) rb_max(tree)) == 0) max_changed = true;
	if (max_changed) rb_max(tree) = rb_prev(rb_max(tree));

	/* delete, update references, do whatever you need to do */
    rb_iterator_t next_node = rb_tree_delete_at((rb_tree_t *) tree, node, copy);

	/* then update the max and min*/
    if (rb_is_empty(tree)) {
		rb_min(tree) = NULL;
		rb_max(tree) = NULL;
    }

	return next_node;
}

/**
 * @fn rb_tree_delete
 * @brief Deletes a node from an rbtree after finding it manually.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[in] cmp Comparator callback used to traverse the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 */
void rb_tree_delete(rb_tree_t *tree, rb_node_t *node, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst)) {
	RB_NULL_CHECK(tree);
	RB_NULL_CHECK(node);
	RB_NULL_CHECK(cmp);
	RB_NULL_CHECK(copy);

	rb_node_t *target;

	/* make sure the node exists */	
    target = (rb_node_t *) rb_find(tree, node, cmp);
    RB_NULL_CHECK(target);

    rb_tree_delete_at(tree, target, copy);
}

/**
 * @fn rb_tree_lcached_delete
 * @brief Deletes a node from an rbtree after finding it manually. Updates the min.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[in] cmp Comparator callback used to traverse the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 */
void rb_tree_lcached_delete(rb_tree_lcached_t *tree, rb_node_t *node, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst)) {
	RB_NULL_CHECK(tree);
	RB_NULL_CHECK(node);
	RB_NULL_CHECK(cmp);
	RB_NULL_CHECK(copy);
	
	rb_node_t *target;

	/* make sure the node exists */	
    target = (rb_node_t *) rb_find((rb_tree_t *) tree, node, cmp);
    RB_NULL_CHECK(target);

    rb_tree_lcached_delete_at(tree, target, cmp, copy);
}

/**
 * @fn rb_tree_rcached_delete
 * @brief Deletes a node from an rbtree after finding it manually. Updates the max.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[in] cmp Comparator callback used to traverse the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 */
void rb_tree_rcached_delete(rb_tree_rcached_t *tree, rb_node_t *node, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst)) {
	RB_NULL_CHECK(tree);
	RB_NULL_CHECK(node);
	RB_NULL_CHECK(cmp);
	RB_NULL_CHECK(copy);

	rb_node_t *target;

	/* make sure the node exists */	
    target = (rb_node_t *) rb_find((rb_tree_t *) tree, node, cmp);
    RB_NULL_CHECK(target);

    rb_tree_rcached_delete_at(tree, target, cmp, copy);
}

/**
 * @fn rb_tree_lrcached_delete
 * @brief Deletes a node from an rbtree after finding it manually. Updates the min and the max.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[in] cmp Comparator callback used to traverse the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 */
void rb_tree_lrcached_delete(rb_tree_lrcached_t *tree, rb_node_t *node, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst)) {
	RB_NULL_CHECK(tree);
	RB_NULL_CHECK(node);
	RB_NULL_CHECK(cmp);
	RB_NULL_CHECK(copy);

	rb_node_t *target;

	/* make sure the node exists */	
    target = (rb_node_t *) rb_find((rb_tree_t *) tree, node, cmp);
    RB_NULL_CHECK(target);

    rb_tree_lrcached_delete_at(tree, target, cmp, copy);
}

/** @} */

/**
 * @defgroup rb_search Red-black tree search function.
 * @{
 */

/**
 * @brief Binary search to find 'key'. Returns NULL if not found.
 * @param[in] anchor Root of the subtree to search.
 * @param[in] key Pointer to the node to be found.
 * @param[in] cmp Comparator callback used for the search.
 */
static inline const rb_node_t *__rb_find(const rb_node_t *anchor, const rb_node_t *key, int (*cmp)(const rb_node_t *left, const rb_node_t *right)) {
	RB_NULL_CHECK(anchor, NULL);
	RB_NULL_CHECK(key, NULL);
	RB_NULL_CHECK(cmp, NULL);

	/* --- */
	
	rb_iterator_t cursor = (rb_iterator_t) anchor;
    rb_iterator_t next = NULL;

	/* perform the same kind of traversal as __rb_insert_basic() */
    while (cursor != NULL) {	
		int comparison = cmp((const rb_node_t *) key, (const rb_node_t *) cursor);
		if (comparison < 0) {			/* left */
			next = rb_left(cursor);
		} else if (comparison == 0) {	/* equal */
			break;
		} else {						/* right */
			next = rb_right(cursor);
		}
		
		cursor = next;
	}

    return cursor;
}

/* --- */

/**
 * @fn rb_find
 * @brief Binary search to find 'key'. Returns NULL if not found.
 * @param[in] tree Root of the full tree to search.
 * @param[in] key Pointer to the node to be found.
 * @param[in] cmp Comparator callback used for the search.
 */
const rb_iterator_t rb_find(const rb_tree_t *tree, const rb_node_t *key, int (*cmp)(const rb_node_t *left, const rb_node_t *right)) {
    return __rb_find(rb_root(tree), key, cmp);
}

/** @} */

/**
 * @defgroup rb_loop Red-black iteration functions.
 * @{
 */

/**
 * @brief Returns the minimum of the subtree rooted at anchor.
 * @param[in] anchor Root of the subtree to search.
 */
static inline const rb_iterator_t __rb_first(const rb_node_t *anchor) {
	RB_NULL_CHECK(anchor, NULL);

    rb_iterator_t cursor = (rb_iterator_t) anchor;
    while (rb_left(cursor) != NULL) {
        cursor = rb_left(cursor);
    }

    return cursor;
}

/**
 * @brief Returns the maximum of the subtree rooted at anchor.
 * @param[in] anchor Root of the subtree to search.
 */
static inline const rb_iterator_t __rb_last(const rb_node_t *anchor) {
	RB_NULL_CHECK(anchor, NULL);

    rb_iterator_t cursor = (rb_iterator_t) anchor;
    while (rb_right(cursor) != NULL) {
        cursor = rb_right(cursor);
    }

    return cursor;
}

/**
 * @fn rb_first
 * @brief Returns the minimum of the subtree rooted at anchor.
 * @param[in] tree Root of the full tree to search.
 */
const rb_iterator_t rb_first(const rb_tree_t *tree) {
    RB_NULL_CHECK(tree, NULL);
    return __rb_first(rb_root(tree));
}

/**
 * @fn rb_last
 * @brief Returns the minimum of the subtree rooted at anchor.
 * @param[in] tree Root of the full tree to search.
 */
const rb_iterator_t rb_last(const rb_tree_t *tree) {
    RB_NULL_CHECK(tree, NULL);
    return __rb_last(rb_root(tree));
}

/**
 * @fn rb_next
 * @brief Returns the first node greater than or equal to node.
 * @param[in] node Current position in the tree.
 */
const rb_iterator_t rb_next(const rb_iterator_t node) {
    RB_NULL_CHECK(node, NULL);
    if (rb_is_disconnected(node)) return NULL;

	/* if there is a right subtree to traverse, then go as far left as possible down there. */
    if (rb_right(node)) return __rb_first(rb_right(node));

	/* --- */

	/* else move up until we are the the 'left' of some other node, in which case that node is the next one. */
	rb_iterator_t cursor, cursor_parent;
	cursor = (rb_iterator_t) node;
	cursor_parent = rb_parent(cursor);
	while ((cursor_parent != NULL) && (cursor == rb_right(cursor_parent))) {
		cursor = cursor_parent;
		cursor_parent = rb_parent(cursor);
	}

	return cursor_parent;
}

/**
 * @fn rb_prev
 * @brief Returns the first node less than node.
 * @param[in] node Current position in the tree.
 */
const rb_iterator_t rb_prev(const rb_iterator_t node) {
    RB_NULL_CHECK(node, NULL);
    if (rb_is_disconnected(node)) return NULL;

	/* if there is a left subtree to traverse, gotta go as far right as possible on that side */
    if (rb_left(node)) return __rb_last(rb_left(node));

	/* --- */

	/* else move up until we are on the right side of something */
    rb_node_t *cursor, *cursor_parent;
	cursor = (rb_iterator_t) node;
	cursor_parent = rb_parent(cursor);
    while (cursor_parent != NULL && cursor == rb_left(cursor_parent)) {
		cursor = cursor_parent;
		cursor_parent = rb_parent(cursor);
    }

    return cursor_parent;
}

/** @} */

/**
 * @defgroup rb_traversals Red-black tree traversal functions
 * @{
 */

/**
 * @brief Performs an in-order traversal of the tree and applies cb to its subtrees recursively.
 * @param[in] anchor Subtree to traverse.
 * @param[in] cb Function to apply to each node.
 */
static inline void __rb_inorder_foreach(rb_node_t *anchor, void (*cb)(rb_node_t *key)) {
	RB_NULL_CHECK(cb);
	if (!anchor) return;

    __rb_inorder_foreach(anchor->left, cb);
    cb(anchor);
    __rb_inorder_foreach(anchor->right, cb);
}

/**
 * @brief Performs a post-order traversal of the tree and applies cb to its subtrees recursively.
 * @param[in] anchor Subtree to traverse.
 * @param[in] cb Function to apply to each node.
 */
static inline void __rb_postorder_foreach(rb_node_t *anchor, void (*cb)(rb_node_t *key)) {
	RB_NULL_CHECK(cb);
	if (!anchor) return;

    __rb_postorder_foreach(anchor->left, cb);
    __rb_postorder_foreach(anchor->right, cb);
    cb(anchor);
}

/**
 * @brief Performs a pre-order traversal of the tree and applies cb to its subtrees recursively.
 * @param[in] anchor Subtree to traverse.
 * @param[in] cb Function to apply to each node.
 */
static inline void __rb_preorder_foreach(rb_node_t *anchor, void (*cb)(rb_node_t *key)) {
	RB_NULL_CHECK(cb);
	if (!anchor) return;

    cb(anchor);
    __rb_preorder_foreach(anchor->left, cb);
    __rb_preorder_foreach(anchor->right, cb);
}

/* --- */

/**
 * @fn rb_inorder_foreach
 * @brief Performs an in-order traversal of the tree and applies cb to every node.
 * @param[in] tree Full tree to traverse.
 * @param[in] cb Function to apply to each node.
 */
void rb_inorder_foreach(rb_tree_t *tree, void (*cb)(rb_node_t *key)) {
	RB_NULL_CHECK(tree);
	RB_NULL_CHECK(cb);

    __rb_inorder_foreach(rb_root(tree), cb);
}

/**
 * @fn rb_preorder_foreach
 * @brief Performs a pre-order traversal of the tree and applies cb to every node.
 * @param[in] tree Full tree to traverse.
 * @param[in] cb Function to apply to each node.
 */
void rb_preorder_foreach(rb_tree_t *tree, void (*cb)(rb_node_t *key)) {
	RB_NULL_CHECK(tree);
	RB_NULL_CHECK(cb);

    __rb_preorder_foreach(rb_root(tree), cb);
}

/**
 * @fn rb_postorder_foreach
 * @brief Performs a post-order traversal of the tree and applies cb to every node.
 * @param[in] tree Full tree to traverse.
 * @param[in] cb Function to apply to each node.
 */
void rb_postorder_foreach(rb_tree_t *tree, void (*cb)(rb_node_t *key)) {
	RB_NULL_CHECK(tree);
	RB_NULL_CHECK(cb);

    __rb_postorder_foreach(rb_root(tree), cb);
}

/** @} */
