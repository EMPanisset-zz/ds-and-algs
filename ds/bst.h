#ifndef __BST__H__
#define __BST__H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stddef.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct bst_node bst_node_t;

struct bst_node {
    bst_node_t *parent;
    bst_node_t *left;
    bst_node_t *right;
    void *data;
    int size; /**< total number of nodes considering this node itself, left subtree and right subtree: useful to compute the kth order statistic */
};

typedef enum bst_type bst_type_t;

enum bst_type {
     BST_T_UNBALANCED
    ,BST_T_AVL
    ,BST_T_SPLAY
};

typedef struct bst bst_t;
typedef int (*bst_cmp_t)(const void *key, const void *value);
typedef void (*bst_free_t)(void *data);

typedef enum bst_order bst_order_t;

enum bst_order {
     BST_PREORDER
    ,BST_INORDER
    ,BST_POSTORDER
};

typedef enum bst_node_type bst_node_type_t;

enum bst_node_type {
     BST_NONE_NODE
    ,BST_ROOT_NODE
    ,BST_LEFT_CHILD_NODE
    ,BST_RIGHT_CHILD_NODE
};

bst_node_type_t
bst_node_type_get(const bst_node_t *node);

void
bst_node_free(bst_t *bst, bst_node_t *node);

bst_t *
bst_new(bst_type_t type, bst_cmp_t cmp, bst_free_t free);

void
bst_free(bst_t *bst);

bst_node_t *
bst_root(bst_t *bst);

void
bst_cmp_set(bst_t *bst, bst_cmp_t cmp);

typedef void (*bst_traverse_t)(void *data, void *arg);

void
bst_traverse(bst_node_t *root, bst_order_t order, bst_traverse_t fn, void *arg);

void
bst_range(bst_t *bst, void *key_low, void *key_high, bst_traverse_t fn, void *arg);

bst_node_t *
bst_kth_order_statistic(bst_t *bst, int k);

bool
is_bst(bst_t *bst);

bst_node_t *
bst_first(bst_t *bst);

bst_node_t *
bst_last(bst_t *bst);

bst_node_t *
bst_find(bst_t *bst, void *key);

bst_node_t *
bst_parent_find(bst_t *bst, void *key);

bst_node_t *
bst_next(bst_t *bst, bst_node_t *node);

bst_node_t *
bst_previous(bst_t *bst, bst_node_t *node);

bst_node_t *
bst_parent(bst_t *bst, void *key);

bst_node_t *
bst_insert(bst_t *bst, void *data, void *key);

bst_node_t *
bst_remove(bst_t *bst, void *key);

void
bst_merge(bst_t *bst, bst_t *left, bst_t *right);

void
bst_split(bst_t *bst, bst_t *left, bst_t *right, void *key);

#endif /* __BST__H__ */
