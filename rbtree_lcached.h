/**
 * @file rbtree_lcached.h
 * @author krad2
 * @date June 28 2020
 * @brief A C red-black tree that caches the minimum element.
 * @see rbtree.h
 */

#ifndef RBTREE_LCACHED_H_
#define RBTREE_LCACHED_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "rbtree.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup rb_structures Structure definitions for the library
 * @{
 */

/**
 * @struct rb_tree_lcached
 * @brief Red-black tree wrapper class; additionally caches the logical min of the tree.
 * @var rb_tree::root
 * Pointer to the root of the actual tree.
 * @var rb_tree::min
 * Minimum element in the tree.
 */
typedef struct rb_tree_lcached {
	rb_node_t *root;
    rb_iterator_t min;	
} rb_tree_lcached_t;

/** @} */

/**
 * @defgroup rb_macros lcached accessor macros.
 * @{
 */

/** Fetches the logical min of the tree. */
#define rb_min(root)   										(root)->min

/** @} */

/**
 * @defgroup rb_api	Red-black tree API.
 * @{
 */

/**
 * @fn rb_tree_lcached_init
 * @details Initializes the tree, then sets the min to NULL.
 * @param[in] tree Pointer to a min-cached rbtree.
 */
void rb_tree_lcached_init(rb_tree_lcached_t *tree);

/**
 * @fn rb_tree_lcached_insert_at
 * @brief Inserts a node into an rb_tree as close as possible to and after the provided iterator. Updates the min.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Pointer to an rb_node instance embedded in something else.
 * @param[in] hint Pointer to a valid rb_node used as a starting position for the insertion.
 * @param[in] cmp Comparator callback used to traverse the tree.
 */
void rb_tree_lcached_insert_at(rb_tree_lcached_t *tree, rb_node_t *node, rb_iterator_t hint, int (*cmp)(const rb_node_t *left, const rb_node_t *right));

/**
 * @fn rb_tree_lcached_insert
 * @brief Inserts a node into an rb_tree, guided by a comparator. Maintains and updates a running min.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Pointer to an rb_node instance embedded in something else.
 * @param[in] cmp Comparator callback used to traverse the tree.
 */
void rb_tree_lcached_insert(rb_tree_lcached_t *tree, rb_node_t *node, int (*cmp)(const rb_node_t *left, const rb_node_t *right));

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
rb_iterator_t rb_tree_lcached_delete_at(rb_tree_lcached_t *tree, rb_iterator_t node, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst));

/**
 * @fn rb_tree_lcached_delete
 * @brief Deletes a node from an rbtree after finding it manually. Updates the min.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[in] cmp Comparator callback used to traverse the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 */
void rb_tree_lcached_delete(rb_tree_lcached_t *tree, rb_node_t *node, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst));

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* RB_TREE_RCACHED_H_ */