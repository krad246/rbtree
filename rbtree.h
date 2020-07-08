/**
 * @file rbtree.h
 * @author krad2
 * @date June 28 2020
 * @brief A red-black implementation in C, defining an intrusive node-based red-black tree class library.
 * @see https://en.wikipedia.org/wiki/Red%E2%80%93black_tree
 */

#ifndef RBTREE_H_
#define RBTREE_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Undefine this if you want to get rid of the tree checks and safety features.
 */
#ifndef RB_UNSAFE
#error "RB_UNSAFE build configuration option undefined"
#endif

/**
 * @defgroup rb_structures Structure definitions for the library
 * @{
 */

/**
 * @enum rb_color 
 * @brief Red-black tree node colors.
 */
typedef enum rb_color {
	rb_red = 0,
	rb_black = 1
} rb_color_t;

/**
 * @struct rb_node
 * @brief A red-black tree node meant to be integrated into a tree structure.
 * @var rb_node::__rb_parent_color
 * Pointer to a parent node combined with a color bit in the LSB.
 * @var rb_node::left
 * Pointer to the left subtree.
 * @var rb_node::right
 * Pointer to the right subtree.
 */
typedef struct __attribute__((aligned(sizeof(uintptr_t)))) rb_node {
   	uintptr_t __rb_parent_color;
    struct rb_node *left;
    struct rb_node *right;
} rb_node_t;

typedef rb_node_t *rb_iterator_t;

/**
 * @struct rb_tree
 * @brief Wrapper class for a tree made of red-black nodes.
 * @var rb_tree::root
 * Pointer to the root of the actual tree.
 */
typedef struct rb_tree {
    rb_node_t *root;
} rb_tree_t;

/** @} */

/**
 * @cond PRIVATE
 * @brief Private macros used for the general purpose ones defined in @ref rb_macros "rb_macros".
 * @{
 */

/** Defines the address alignment of a node */
#define __rb_node_alignment									(sizeof(uintptr_t))

/** Determines the number of free bits in a node pointer for use as a color bit */
#define __rb_color_mask										(__rb_node_alignment - 1)

/** Masks the parent bits out in the parent pointer to yield the color bit */
#define __rb_color(pc)     									((rb_color_t) ((pc) & (__rb_color_mask)))

/** Checks that the lowest bit is rb_black, i.e. 1. */
#define __rb_is_black(pc)  									(__rb_color((pc)) == rb_black)

/** Checks that the lowest bit is rb_red, i.e. 0. */
#define __rb_is_red(pc)    									(__rb_color((pc)) == rb_red)

/** 
 * Masks out the color bits and extra bits to guarantee pointer alignment to uintptr_t, 
 * since an rb_node_t is aligned to uintptr_t boundaries. 
 */
#define __rb_parent(pc)    									((rb_node_t *) ((pc) & ~(__rb_color_mask)))

/**
 * Computes the offset the member has in a structure of that type, 
 * then moves the pointer back by that much. 
 */
#define container_of(ptr, type, member) 					({                      									\
        														const typeof(((type *) 0)->member) *__mptr = (ptr);    	\
       			 												(type *)((char *) __mptr - offsetof(type, member));		\
															})	

/** 
 * @defgroup rb_check Helper pointer check macros
 * @{
 */

/* Base case pointer checker with no return value */
#define __RB_NULL_CHECK(ptr)								do {\
																if (!RB_UNSAFE)	{					\
																	if (!(ptr)) {					\
																		return;						\
																	}								\
																}									\
															} while (0)				

/* Base case pointer checker with return value */
#define __RB_NULL_CHECK_V(ptr, ret)							do {\
																if (!RB_UNSAFE)	{					\
																	if (!(ptr)) {					\
																		return (ret);				\
																	}								\
																}									\
															} while (0)				


#define __RB_DISC_CHECK(ptr)								do {\
																if (!RB_UNSAFE)	{					\
																	if (rb_is_disconnected(ptr)) {	\
																		return;						\
																	}								\
																}									\
															} while (0)				


#define __RB_DISC_CHECK_V(ptr, ret)							do {\
																if (!RB_UNSAFE)	{					\
																	if (rb_is_disconnected(ptr)) {	\
																		return (ret);				\
																	}								\
																}									\
															} while (0)								

/* 
 * Use the variadic arguments to change _FUNC to the appropriate function,
 * i.e. a single argument x shows up with _1 = x, _2 = __RB_NULL_CHECK_V, and 
 * _FUNC = __RB_NULL_CHECK, which returns _FUNC to __RB_NULL_CHECK, subsequently
 * invoked with x as the argument. 
 * 
 * Similarly, passing 2 arguments slides _FUNC over to cover __RB_NULL_CHECK_V,
 * returning the version of RB_NULL_CHECK that accepts a return value.
 * 
 * More than 2 arguments and _FUNC is no longer defined, allowing for type safety.
 */

#define __RB_NULL_CHECK_S(_1, _2, _FUNC, ...) 				_FUNC
#define RB_NULL_CHECK(...) 									__RB_NULL_CHECK_S(__VA_ARGS__, __RB_NULL_CHECK_V, __RB_NULL_CHECK)(__VA_ARGS__)

/* Same thing, but to test for disconnected nodes */
#define __RB_DISC_CHECK_S(_1, _2, _FUNC, ...) 				_FUNC
#define RB_DISC_CHECK(...) 									__RB_DISC_CHECK_S(__VA_ARGS__, __RB_DISC_CHECK_V, __RB_DISC_CHECK)(__VA_ARGS__)

/** @} */														

/** 
 * @endcond
 * @} 
 */

/**
 * @defgroup rb_macros General purpose red-black tree macros.
 * @{
 */

/**
 * @brief Tree interface macros to fetch the min, max, or the tree instance itself, if they exist.
 */

/** Fetches the root node of the tree. */
#define rb_root(tree)        								((tree)->root)

/** Asserts if the tree root is null. */
#define rb_is_empty(tree)  									(rb_root((tree)) == NULL)

/**
 * @brief Node color macros to fetch the color or check against a color.
 */

/** Returns rb_black if the pointer is NULL or it is defined as a black node, else red. */
#define rb_color(rb)       									((rb_color_t) (((rb) == NULL) || __rb_color((rb)->__rb_parent_color)))

/** Returns true if the node is defined as red and is not a NULL leaf. */
#define rb_is_red(rb)      									((bool) (((rb) != NULL) && __rb_is_red((rb)->__rb_parent_color)))

/** Returns true if the node is defined as black or is a NULL leaf. */
#define rb_is_black(rb)    									((bool) ((rb) == NULL) || __rb_is_black((rb)->__rb_parent_color))

/**
 * @brief Node link macros to get and set a node's graph connections.
 */

/** Returns the parent node of rb. */
#define rb_parent(rb)										__rb_parent((rb)->__rb_parent_color)

/** Returns the left child / subtree of rb. */
#define rb_left(rb)         								((rb)->left)

/** Returns the right child / subtree of rb. */
#define rb_right(rb)        								((rb)->right)

/* --- */

/*
 * A disconnected node is one that loops back on itself and has no subtrees to its name.
 */

/** Disconnects rb from its parent and its subtrees. */
#define rb_disconnect(rb)									({														\
																(rb)->__rb_parent_color = (uintptr_t) (rb);			\
																rb_left(rb) = NULL;									\
																rb_right(rb) = NULL;								\
															})

/** Returns true if rb loops back on itself and is isolated from all other nodes. */
#define rb_is_disconnected(rb)								(rb_parent((rb)) == (rb) && !rb_left(rb) && !rb_right(rb))

/**
 * @brief Intrustive node macros to access the containing object of an rb_node.
 */

/** 
 * Performs a null pointer check to avoid a crash from an illegal memory access. 
 * Returns the containing object if possible.
 */
#define rb_entry_safe(ptr, type, member)					({ 															\
																typeof(ptr) ____ptr = (ptr); 							\
																____ptr ? container_of(____ptr, type, member) : NULL;	\
															})

/*
 * Accessing the container defaults to the safety setting of the library.
 */

#if (RB_UNSAFE == 1)
	#define rb_entry(ptr, type, member)		 				container_of(ptr, type, member)
#else
	#define rb_entry(ptr, type, member)						rb_entry_safe(ptr, type, member)
#endif

/** @} */

/**
 * @defgroup rb_api	Red-black tree API.
 * @{
 */

/**
 * @fn rb_tree_init
 * @brief Initializes an rb_tree.
 * @param[in] tree Pointer to an rb_tree instance.
 */
void rb_tree_init(rb_tree_t *tree);

/**
 * @fn rb_tree_insert_at
 * @brief Inserts a node into an rb_tree as close as possible to and after the provided iterator.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Pointer to an rb_node instance embedded in something else.
 * @param[in] hint Pointer to a valid rb_node used as a starting position for the insertion.
 * @param[in] cmp Comparator callback used to traverse the tree.
 */
void rb_tree_insert_at(rb_tree_t *tree, rb_node_t *node, rb_iterator_t hint, int (*cmp)(const rb_node_t *left, const rb_node_t *right));

/**
 * @fn rb_tree_insert
 * @brief Inserts a node into an rb_tree, guided by a comparator.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Pointer to an rb_node instance embedded in something else.
 * @param[in] cmp Comparator callback used to traverse the tree.
 */
void rb_tree_insert(rb_tree_t *tree, rb_node_t *node, int (*cmp)(const rb_node_t *left, const rb_node_t *right));

/**
 * @fn rb_tree_delete_at
 * @brief Deletes a node from a rbtree at an iterator.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[out] deleted Pointer to the node that was removed from the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 * @return Iterator to the next element.
 */
rb_iterator_t rb_tree_delete_at(rb_tree_t *tree, rb_iterator_t node, rb_iterator_t *deleted, void (*copy)(const rb_node_t *src, rb_node_t *dst));

/**
 * @fn rb_tree_delete
 * @brief Deletes a node from an rbtree after finding it manually.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] node Iterator into the tree.
 * @param[in] hint Pointer to a valid rb.
 * @param[out] deleted Node that was deleted.
 * @param[in] cmp Comparator callback used to traverse the tree.
 * @param[in] copy Copy callback used for successor node deletion.
 * @return Iterator to the next element.
 */
rb_iterator_t rb_tree_delete(rb_tree_t *tree, rb_node_t *node, rb_iterator_t *deleted, int (*cmp)(const rb_node_t *left, const rb_node_t *right), void (*copy)(const rb_node_t *src, rb_node_t *dst));

/**
 * @fn rb_find
 * @brief Searches the tree for a node and returns an iterator to it.
 * @param[in] tree Pointer to an rb_tree instance.
 * @param[in] key Pointer to a node key to be searched.
 * @oaran[in] cmp Comparator callback used to traverse the tree.
 */
const rb_iterator_t rb_find(const rb_tree_t *tree, const rb_node_t *key, int (*cmp)(const rb_node_t *left, const rb_node_t *right));

/**
 * @fn rb_first
 * @brief Manually returns a pointer to the logical min of the tree.
 * @param[in] tree Pointer to an rb_tree instance.
 */
const rb_iterator_t rb_first(const rb_tree_t *tree);

/**
 * @fn rb_last
 * @brief Manually returns a pointer to the logical max of the tree.
 * @param[in] tree Pointer to an rb_tree instance.
 */
const rb_iterator_t rb_last(const rb_tree_t *tree);

/**
 * @fn rb_next
 * @brief Returns an iterator to the next element, in sorted order, in the tree.
 * @param[in] node Valid iterator into the tree.
 */
const rb_iterator_t rb_next(const rb_iterator_t node);

/**
 * @fn rb_prev
 * @brief Returns an iterator to the previous element, in sorted order, in the tree.
 * @param[in] node Valid iterator into the tree.
 */
const rb_iterator_t rb_prev(const rb_iterator_t node);

/**
 * @brief Performs an in-order traversal of the tree and applies cb to every node.
 * @param[in] tree Full tree to traverse.
 * @param[in] cb Function to apply to each node.
 */
void rb_inorder_foreach(rb_tree_t *tree, void (*cb)(rb_node_t *key));

/**
 * @brief Performs a post-order traversal of the tree and applies cb to every node.
 * @param[in] tree Full tree to traverse.
 * @param[in] cb Function to apply to each node.
 */
void rb_postorder_foreach(rb_tree_t *tree, void (*cb)(rb_node_t *key));

/**
 * @brief Performs a pre-order traversal of the tree and applies cb to every node.
 * @param[in] tree Full tree to traverse.
 * @param[in] cb Function to apply to each node.
 */
void rb_preorder_foreach(rb_tree_t *tree, void (*cb)(rb_node_t *key));

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* RB_TREE_H_ */
