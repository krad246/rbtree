#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#include "rbtree.h"


#define max(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#define min(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; })

struct a {
    int x;
    rb_node y;
};

int cmp(const void *left, const void *right) {
    struct a *l = rb_entry_safe(left, struct a, y);
    struct a *r = rb_entry_safe(right, struct a, y);

	if (l == r) return 0;
    return l->x - r->x;
}

#define node_cnt 25000
volatile int sort_tst[node_cnt];
volatile int q_tst[node_cnt];
struct a buf[node_cnt];
int q2_tst[node_cnt];

int p = 0;
void cb(void *key) {
    struct a *k = rb_entry_safe(key, struct a, y);
	sort_tst[p] = k->x;
	q_tst[p] = k->x;
	p++;
}

void copy(const void *src, void *dst) {
    struct a *l = rb_entry_safe(src, struct a, y);
    struct a *r = rb_entry_safe(dst, struct a, y);

    r->x = l->x;
}

bool __rb_height_balance_tst(rb_node *root, 
	unsigned int *max_height, unsigned int *min_height) { 

    // Base case - null node has no height and is degenerately balanced
    if (root == NULL) { 
        *min_height = 0;
		*max_height = 0; 
        return true; 
    } 

	// recursively check the balance of the subtrees on each side
    unsigned int left_max_height, left_min_height;
    unsigned int right_max_height, right_min_height;

	left_max_height = 0;
	right_max_height = 0;

	left_min_height = 0;
	right_min_height = 0;

    if (!__rb_height_balance_tst(root->left, &left_max_height, &left_min_height)) {
		return false;
	}

	if (!__rb_height_balance_tst(root->right, &right_max_height, &right_min_height)) {
		return false; 
	}
  
    // Set the max and min heights of this node for the parent call 
    *max_height = (max(left_max_height, right_max_height)) + 1; 
    *min_height = (min(left_min_height, right_min_height)) + 1; 

    // See if this parent node is balanced by checking that the maximum height (path from root to farthest leaf) is AT MOST twice as deep as the minimum height (path from root to closest leaf)
    if ((*max_height) <= 2 * (*min_height)) return true; 
	else return false; 
} 

// Wrapper over the actual recursive routine
bool rb_is_balanced(rb_tree *tree) {
	unsigned int __max_height_internal, __min_height_internal;
	__min_height_internal = 0;
	__max_height_internal = 0;
	
	bool rb_root_balanced = __rb_height_balance_tst(rb_root(tree), &__max_height_internal, &__min_height_internal);
	return rb_root_balanced;
}


int main(int argc, char **argv) {
	srand(time(NULL));

	rb_tree_rcached t;
	rb_tree_init(&t.tree);
	t.rightmost = NULL;

	int max_so_far = INT32_MIN;

	for (int i = 0; i < node_cnt; ++i) {
		buf[i].x = rand() % node_cnt;
		if (buf[i].x > max_so_far) max_so_far = buf[i].x;

		rb_node_init(&buf[i].y);
	}

	clock_t begin = clock();

	for (int i = 0; i < node_cnt; ++i) {
		rb_insert_rcached(&t, &buf[i].y, cmp);
		assert(rb_find(&t.tree, &buf[i].y, cmp) != NULL);
	}

	clock_t end = clock();
	
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

	printf("%f\n", time_spent);
	
	rb_node *itq = (rb_node *) rb_next(rb_first(&t.tree));
	rb_node *itm = (rb_node *) rb_first(&t.tree);

	rb_node *itu = (rb_node *) rb_prev(rb_last(&t.tree));
	rb_node *ite = (rb_node *) rb_last(&t.tree);
	assert (rb_entry_safe(ite, struct a, y)->x == max_so_far);

	rb_inorder_foreach(&t.tree, cb);
	for (int j = 1; j < node_cnt; ++j) {
		int ui = rb_entry_safe(itu, struct a, y)->x;
		int ue = rb_entry_safe(ite, struct a, y)->x;
		int uq = rb_entry_safe(itq, struct a, y)->x;
		int um = rb_entry_safe(itm, struct a, y)->x;

		assert(ui<= ue);
		assert(uq >= um);
		assert(sort_tst[j] >= sort_tst[j - 1]);

		itq = (rb_node *) rb_next(itq);
		itm = (rb_node *) rb_next(itm);

		itu = (rb_node *) rb_prev(itu);
		ite = (rb_node *) rb_prev(ite);
	}

	assert(rb_is_balanced(&t.tree));
	
	memset(sort_tst, 0, sizeof(*sort_tst) * node_cnt);
	memset(q2_tst, 0, sizeof(*q_tst) * node_cnt);
	for (int i = 0; i < node_cnt; ++i) {
		sort_tst[buf[i].x]++;
		q2_tst[buf[i].x]++;
		
	}

	for (int i = 0; i < node_cnt; ++i) {
		printf("%d: %d\n", buf[i].x, q2_tst[buf[i].x]);
		assert(rb_find(&t.tree, &buf[i].y, cmp) != NULL);
	}

	assert(rb_is_balanced(&t.tree));
	rb_node *orig_root = t.tree.node;

	for (int i = 0; i < node_cnt; ++i) {
		q2_tst[i] = buf[i].x;
	}

	// delete a random 50% of the array, then reinsert a new 50% in place of it
	for (int i = 0; i < node_cnt; ++i) {
		// bool randbool = rand() & 1;
		
		struct a item_to_delete;
		item_to_delete.x = q2_tst[i];
		// printf("round %d: delete %d\n", i, item_to_delete.x);

		sort_tst[item_to_delete.x]--;
		// printf("count %d\n", sort_tst[item_to_delete.x]);

		// if (t.tree.node) printf("root %d\n", rb_entry_safe(t.tree.node, struct a, y)->x);

		rb_delete_rcached(&t, &item_to_delete.y, cmp, copy);
		rb_node *l = (rb_node *) rb_last(&t.tree);

		// printf("checking that caching max works as intended \n");
		assert(cmp(l, rb_last_cached(&t)) == 0);
		// if (t.tree.node) printf("root %d\n", rb_entry_safe(t.tree.node, struct a, y)->x);
		
		
		// assert(rb_is_balanced(&t.tree));
		// printf("balanced\n");

		if (sort_tst[item_to_delete.x] == 0) {
			volatile rb_node  *sksd = rb_find(&t.tree, &item_to_delete.y, cmp);
			const struct a *q = rb_entry_safe(sksd, struct a, y);
			// printf("checking if all instances are removed \n");
			assert(sksd == NULL);
			// printf("deletion successful, no elements left \n");
		} else {
			volatile rb_node  *sksd = rb_find(&t.tree, &item_to_delete.y, cmp);
			const struct a *q = rb_entry_safe(sksd, struct a, y);
			// printf("checking for remaining instances \n");
			assert(sksd != NULL);
			assert(cmp(sksd, &item_to_delete.y) == 0);
			// printf("deletion successful, some elements left \n");
		}


		// if (randbool) {
		// 	buf[i].x = rand() % (node_cnt * node_cnt);
		// }

		// rb_insert_rcached(&t, &buf[i].y, cmp);
		// assert(rb_is_balanced(&t.tree));
	}

    return 0;
}