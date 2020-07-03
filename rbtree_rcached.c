/**
 * @file rbtree_rcached.c
 * @author krad2
 * @date June 28 2020
 * @brief A C red-black tree that caches the maximum element.
 * @see rbtree.c
 */

#include "rbtree_rcached.h"

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

	rb_tree_insert_at((rb_tree_t *) tree, node, hint, cmp);
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
 * @fn rb_tree_rcached_delete_at
 * @brief Deletes a node from a rbtree at an iterator. Updates the max.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[out] deleted Node that was deleted.
 * @param[in] cmp Comparator callback used to traverse the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 * @return Iterator to the next element.
 */
rb_iterator_t rb_tree_rcached_delete_at(rb_tree_rcached_t *tree, rb_iterator_t node, rb_iterator_t *deleted, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst)) {

	/* check if the max of the tree changed. if it did, slide the max pointer forward */
    bool max_changed = false;
    if (cmp((const rb_node_t *) node, (const rb_node_t *) rb_max(tree)) == 0) max_changed = true;
	if (max_changed) rb_max(tree) = rb_prev(rb_max(tree));

	/* delete, update references, do whatever you need to do */
    rb_iterator_t next_node = rb_tree_delete_at((rb_tree_t *) tree, node, deleted, copy);

	/* then update the max */
    if (rb_is_empty(tree)) {
		rb_max(tree) = NULL;
    }

	return next_node;
}

/**
 * @fn rb_tree_rcached_delete
 * @brief Deletes a node from an rbtree after finding it manually. Updates the max.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[out] deleted Node that was deleted.
 * @param[in] cmp Comparator callback used to traverse the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 */
void rb_tree_rcached_delete(rb_tree_rcached_t *tree, rb_node_t *node, rb_iterator_t *deleted, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst)) {
	RB_NULL_CHECK(tree);
	RB_NULL_CHECK(node);
	RB_NULL_CHECK(cmp);
	RB_NULL_CHECK(copy);

	rb_node_t *target;

	/* make sure the node exists */	
    target = (rb_node_t *) rb_find((rb_tree_t *) tree, node, cmp);
    RB_NULL_CHECK(target);

    rb_tree_rcached_delete_at(tree, target, deleted, cmp, copy);
}