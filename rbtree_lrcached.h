/**
 * @file rbtree_lrcached.h
 * @author krad2
 * @date June 28 2020
 * @brief A C red-black tree that caches the minimum and maximum element.
 * @see rbtree.h
 */

#ifndef RBTREE_LRCACHED_H_
#define RBTREE_LRCACHED_H_

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
 * @struct rb_tree_lrcached
 * @brief Red-black tree wrapper class; additionally caches the logical min and max of the tree.
 * @var rb_tree::root
 * Pointer to the root of the actual tree.
 * @var rb_tree::min
 * Minimum element in the tree.
 * @var rb_tree::max
 * Maximum element in the tree.
 */
typedef struct rb_tree_lrcached {
    rb_node_t *root;
    rb_iterator_t min;
    rb_iterator_t max;
} rb_tree_lrcached_t;

/** @} */

/**
 * @defgroup rb_macros rcached accessor macros
 * @{
 */

/** Fetches the logical min of the tree. */
#define rb_min(root)   										(root)->min

/** Fetches the logical max of the tree. */
#define rb_max(root)    									(root)->max

/** @} */

/**
 * @defgroup rb_api	Red-black tree API.
 * @{
 */

/**
 * @fn rb_tree_lrcached_init
 * @details Combination of rb_tree_lcached_init and rb_tree_rcached_init
 * @param[in] tree Pointer to a lrcached_rbtree.
 */
void rb_tree_lrcached_init(rb_tree_lrcached_t *tree);

/**
 * @fn rb_tree_lrcached_insert_at
 * @brief Inserts a node into an rb_tree as close as possible to and after the provided iterator. Updates the min and the max.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Pointer to an rb_node instance embedded in something else.
 * @param[in] hint Pointer to a valid rb_node used as a starting position for the insertion.
 * @param[in] cmp Comparator callback used to traverse the tree.
 */
void rb_tree_lrcached_insert_at(rb_tree_lrcached_t *tree, rb_node_t *node, rb_iterator_t hint, int (*cmp)(const rb_node_t *left, const rb_node_t *right));

/**
 * @fn rb_tree_lrcached_insert
 * @brief Combination of rb_tree_rcached_insert and rb_tree_lcached_insert.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Pointer to an rb_node instance embedded in something else.
 * @param[in] cmp Comparator callback used to traverse the tree.
 */
void rb_tree_lrcached_insert(rb_tree_lrcached_t *tree, rb_node_t *node, int (*cmp)(const rb_node_t *left, const rb_node_t *right));

/**
 * @fn rb_tree_lrcached_delete_at
 * @brief Deletes a node from a rbtree at an iterator. Updates the min and the max.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[out] deleted Node that was deleted.
 * @param[in] cmp Comparator callback used to traverse the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 * @return Iterator to the next element.
 */
rb_iterator_t rb_tree_lrcached_delete_at(rb_tree_lrcached_t *tree, rb_iterator_t node, rb_iterator_t *deleted, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst));

/**
 * @fn rb_tree_lrcached_delete
 * @brief Deletes a node from an rbtree after finding it manually. Updates the min and the max.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[out] deleted Node that was deleted.
 * @param[in] cmp Comparator callback used to traverse the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 */
void rb_tree_lrcached_delete(rb_tree_lrcached_t *tree, rb_node_t *node, rb_iterator_t *deleted, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst));

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* RB_TREE_LRCACHED_H_ */