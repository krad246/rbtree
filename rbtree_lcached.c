/**
 * @file rbtree_lcached.h
 * @author krad2
 * @date June 28 2020
 * @brief A C red-black tree that caches the minimum element.
 * @see rbtree.h
 */

#include "rbtree_lcached.h"

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

	rb_tree_insert_at((rb_tree_t *) tree, node, hint, cmp);
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
 * @fn rb_tree_lcached_delete_at
 * @brief Deletes a node from a rbtree at an iterator. Updates the min.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[out] deleted Node that was deleted.
 * @param[in] cmp Comparator callback used to traverse the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 * @return Iterator to the next element.
 */
rb_iterator_t rb_tree_lcached_delete_at(rb_tree_lcached_t *tree, rb_iterator_t node, rb_iterator_t *deleted, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst)) {

	/* check if the min of the tree changed. if it did, slide the min pointer forward */
    bool min_changed = false;
    if (cmp((const rb_node_t *) node, (const rb_node_t *) rb_min(tree)) == 0) min_changed = true;
	if (min_changed) rb_min(tree) = rb_next(node);

	/* delete, update references, do whatever you need to do */
    rb_iterator_t next_node = rb_tree_delete_at((rb_tree_t *) tree, node, deleted, copy);
    
	/* then update the min */
	if (rb_is_empty(tree)) {
		rb_min(tree) = NULL;
    }

	return next_node;
}

/**
 * @fn rb_tree_lcached_delete
 * @brief Deletes a node from an rbtree after finding it manually. Updates the min.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[out] deleted Node that was deleted.
 * @param[in] cmp Comparator callback used to traverse the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 * @return Iterator to the next element.
 */
rb_iterator_t rb_tree_lcached_delete(rb_tree_lcached_t *tree, rb_node_t *node, rb_iterator_t *deleted, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst)) {
	RB_NULL_CHECK(tree);
	RB_NULL_CHECK(node);
	RB_NULL_CHECK(cmp);
	RB_NULL_CHECK(copy);
	
	rb_node_t *target;

	/* make sure the node exists */	
    target = (rb_node_t *) rb_find((rb_tree_t *) tree, node, cmp);
    RB_NULL_CHECK(target);

    rb_tree_lcached_delete_at(tree, target, deleted, cmp, copy);
}