/**
 * @file rbtree_rcached.h
 * @author krad2
 * @date June 28 2020
 * @brief A C red-black tree that caches the maximum element.
 * @see rbtree.h
 */

#ifndef RBTREE_RCACHED_H_
#define RBTREE_RCACHED_H_

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
 * @struct rb_tree_rcached
 * @brief Red-black tree wrapper class; additionally caches the logical max of the tree.
 * @var rb_tree::root
 * Pointer to the root of the actual tree.
 * @var rb_tree::max
 * Maximum element in the tree.
 */
typedef struct rb_tree_rcached {
	rb_node_t *root;
    rb_iterator_t max;	
} rb_tree_rcached_t;

/** @} */

/**
 * @defgroup rb_macros rcached accessor macros
 * @{
 */

/** Fetches the logical max of the tree. */
#define rb_max(root)    									(root)->max

/** @} */

/**
 * @defgroup rb_api	Red-black tree API.
 * @{
 */

/**
 * @fn rb_tree_rcached_init
 * @details Initializes the tree, then sets the max to NULL.
 * @param[in] tree Pointer to a max-cached rbtree.
 */
void rb_tree_rcached_init(rb_tree_rcached_t *tree);

/**
 * @fn rb_tree_rcached_insert_at
 * @brief Inserts a node into an rb_tree as close as possible to and after the provided iterator. Updates the max.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Pointer to an rb_node instance embedded in something else.
 * @param[in] hint Pointer to a valid rb_node used as a starting position for the insertion.
 * @param[in] cmp Comparator callback used to traverse the tree.
 */
void rb_tree_rcached_insert_at(rb_tree_rcached_t *tree, rb_node_t *node, rb_iterator_t hint, int (*cmp)(const rb_node_t *left, const rb_node_t *right));

/**
 * @fn rb_tree_rcached_insert
 * @brief Inserts a node into an rb_tree, guided by a comparator. Maintains and updates a running max.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Pointer to an rb_node instance embedded in something else.
 * @param[in] cmp Comparator callback used to traverse the tree.
 */
void rb_tree_rcached_insert(rb_tree_rcached_t *tree, rb_node_t *node, int (*cmp)(const rb_node_t *left, const rb_node_t *right));

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
rb_iterator_t rb_tree_rcached_delete_at(rb_tree_rcached_t *tree, rb_iterator_t node, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst));

/**
 * @fn rb_tree_rcached_delete
 * @brief Deletes a node from an rbtree after finding it manually. Updates the max.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[in] cmp Comparator callback used to traverse the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 */
void rb_tree_rcached_delete(rb_tree_rcached_t *tree, rb_node_t *node, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst));

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* RB_TREE_RCACHED_H_ */