#include "bst.h"

#define downcast(p, type, member)  ((p) ? ((type *)(((char *)(p)) - offsetof(type, member))) : NULL)

typedef struct bst_avl_node bst_avl_node_t;

struct bst_avl_node {
    bst_node_t node;
    int height; /**< used to keep avl tree balanced in O(log(n)) operations */
};

#define bst_max(a, b)   ((a) >= (b) ? (a) : (b))

typedef bst_node_t * (*bst_insert_t)(bst_t *bst, void *data, void *key);
typedef bst_node_t * (*bst_remove_t)(bst_t *bst, void *key);
typedef bst_node_t * (*bst_find_t)(bst_t *bst, void *key);
typedef void (*bst_split_t)(bst_t *bst, bst_t *left, bst_t *right, void *key);
typedef void (*bst_merge_t)(bst_t *bst, bst_t *left, bst_t *right);

struct bst {
    bst_node_t *root;
    bst_cmp_t cmp;
    bst_free_t free;
    bst_find_t find;
    bst_insert_t insert;
    bst_remove_t remove;
    bst_split_t split;
    bst_merge_t merge;
    bst_type_t type;
};

static bst_node_t *
_bst_node_new(bst_type_t type) {
    if (BST_T_AVL == type) {
        bst_avl_node_t *n = calloc(1, sizeof(bst_avl_node_t));
        if (NULL != n) {
            return &n->node;
        }
    }
    bst_node_t *n = calloc(1, sizeof(bst_node_t));
    return n;
}

void
bst_node_free(bst_t *bst, bst_node_t *node) {
    if (NULL != node) {
        node->right = NULL;
        node->left  = NULL;
        node->parent = NULL;
        if (bst->free) {
            bst->free(node->data);
        }
        node->data = NULL;
        free(node);
    }
}

static void
_bst_node_recursive_free(bst_t *bst, bst_node_t *root) {
    if (NULL == root) {
        return;
    }
    _bst_node_recursive_free(bst, root->left);
    _bst_node_recursive_free(bst, root->right);
    bst_node_free(bst, root);
}

bst_node_t *
bst_root(bst_t *bst)
{
    if (NULL != bst) {
        return bst->root;
    }
    return NULL;
}

void
bst_cmp_set(bst_t *bst, bst_cmp_t cmp)
{
    bst->cmp = cmp;
}

void
bst_traverse(bst_node_t *root, bst_order_t order, bst_traverse_t fn, void *arg)
{
    if (NULL == root) {
        return;
    }
    if (order == BST_PREORDER) {
        fn(root->data, arg);
    }
    bst_traverse(root->left, order, fn, arg);
    if (order == BST_INORDER) {
        fn(root->data, arg);
    }
    bst_traverse(root->right, order, fn, arg);
    if (order == BST_POSTORDER) {
        fn(root->data, arg);
    }
}

/**
 * Checks if this tree is a binary search tree.
 *
 * root must be greater than each element in its left subtree and
 * less than or equal each element in its right subtree.
 *
 */
static bool 
_is_bst(bst_t *bst, bst_node_t *root, void **prev) {

    if (NULL == root) {
        return true;
    }

    if (!_is_bst(bst, root->left, prev)) {
        return 0;
    }

    if (*prev != NULL) {
        if (bst->cmp(*prev, root->data) > 0) {
            return false;
        }
    }
    *prev = root->data;

    if (!_is_bst(bst, root->right, prev)) {
        return false;
    }

    return true;
}

bool
is_bst(bst_t *bst) {
    void *prev = NULL;
    return _is_bst(bst, bst->root, &prev);
}

static bst_node_t *
_bst_node_find(bst_t *bst, bst_node_t *root, void *key)
{
    int ret;

    if (NULL == root) {
        return NULL;
    }

    ret = bst->cmp(key, root->data);

    if (0 == ret) {
        return root;
    }

    if (ret < 0) {
        return _bst_node_find(bst, root->left, key);
    }

    return _bst_node_find(bst, root->right, key); 
}

static bst_node_t *
_bst_find(bst_t *bst, void *key)
{
    return _bst_node_find(bst, bst->root, key);
}

bst_node_t *
bst_find(bst_t *bst, void *key)
{
    return bst->find(bst, key);
}

static bst_node_t *
_bst_parent_find(bst_t *bst, bst_node_t *root, void *key)
{
    int ret;

    if (NULL == root) {
        return NULL;
    }

    ret = bst->cmp(key, root->data);

    if (0 == ret) {
        return root;
    }

    if (ret < 0) {
        if (NULL == root->left) {
            return root;
        }
        return _bst_parent_find(bst, root->left, key);
    }

    if (NULL == root->right) {
        return root;
    }
    return _bst_parent_find(bst, root->right, key); 
}

bst_node_t *
bst_parent_find(bst_t *bst, void *key)
{
    return _bst_parent_find(bst, bst->root, key);
}

int
bst_node_height(bst_type_t type, bst_node_t *node)
{
    if (NULL == node) {
        return 0;
    }
    if (BST_T_AVL == type) {
        bst_avl_node_t *avl_node = downcast(node, bst_avl_node_t, node);
        return avl_node->height;
    }
    return 1 + bst_max(bst_node_height(type, node->left), bst_node_height(type, node->right));
}

static void
_bst_path_update(bst_t *bst, bst_node_t *node)
{
    if (NULL != node) {
        node->size   = 1 + (node->left ? node->left->size : 0) + (node->right ? node->right->size : 0);
        if (BST_T_AVL == bst->type) {
            bst_avl_node_t *avl_node = downcast(node, bst_avl_node_t, node);
            avl_node->height = 1 + bst_max(bst_node_height(bst->type, node->left), bst_node_height(bst->type, node->right));
        }
        _bst_path_update(bst, node->parent);
    }
}

static bst_node_t *
_bst_insert(bst_t *bst, void *data, void *key)
{
    bst_node_t *node = NULL;
    bst_node_t *parent = NULL;
    node = _bst_node_new(bst->type);

    if (NULL == node) {
        return NULL;
    }
    node->data = data;

    parent = bst_parent_find(bst, key);
    if (NULL == parent) {
        bst->root = node;
        _bst_path_update(bst, node);
        return node;
    }
    node->parent = parent;

    if (bst->cmp(key, parent->data) < 0) {
        if (NULL != parent->left) {
            parent->left->parent = node;
        }
        node->left = parent->left;
        parent->left = node;
        _bst_path_update(bst, node);
        return node;
    }

    if (NULL != parent->right) {
        parent->right->parent = node;
    }
    node->right = parent->right;
    parent->right = node;
    _bst_path_update(bst, node);
    return node;
}

bst_node_t *
bst_first(bst_t *bst)
{
    if (NULL == bst->root) {
        return NULL;
    }
    
    bst_node_t *first = bst->root;

    while (NULL != first->left) {
        first = first->left;
    }

    return first;    
}

bst_node_t *
bst_last(bst_t *bst)
{
    if (NULL == bst->root) {
        return NULL;
    }
    
    bst_node_t *last = bst->root;

    while (NULL != last->right) {
        last = last->right;
    }

    return last;    
}

bst_node_t *
bst_next(bst_t *bst, bst_node_t *node)
{
    bst_node_t *next = node->right;
    if (NULL != next) {
        while (NULL != next->left) {
            next = next->left;
        }
    }
    else {
        bst_node_type_t node_type = bst_node_type_get(node);
        while (BST_RIGHT_CHILD_NODE == node_type) {
            node = node->parent;
            node_type = bst_node_type_get(node);
        }
        if (BST_LEFT_CHILD_NODE == node_type) {
            next = node->parent;
        }
    }
    return next;
}

bst_node_t *
bst_previous(bst_t *bst, bst_node_t *node)
{
    bst_node_t *previous = node->left;
    if (NULL != previous) {
        while (NULL != previous->right) {
            previous = previous->right;
        }
    }
    else {
        bst_node_type_t node_type = bst_node_type_get(node);
        while (BST_LEFT_CHILD_NODE == node_type) {
            node = node->parent;
            node_type = bst_node_type_get(node);
        }
        if (BST_RIGHT_CHILD_NODE == node_type) {
            previous = node->parent;
        }
    }
    return previous;
}

bst_node_type_t
bst_node_type_get(const bst_node_t *node)
{
    if (NULL != node) {
        bst_node_t *parent = node->parent;

        if (NULL == parent) {
            return BST_ROOT_NODE;
        }
        if (parent->left == node) {
            return BST_LEFT_CHILD_NODE;
        }
        if (parent->right == node) {
            return BST_RIGHT_CHILD_NODE;
        }
    }
    return BST_NONE_NODE; 
}

static void 
_bst_remove_node(bst_t *bst, bst_node_t *node, bst_node_t **next_parent)
{
    bst_node_type_t node_type;
	node_type = bst_node_type_get(node);

    if (NULL != node->right) {
	    bst_node_t *next = bst_next(bst, node);
        bst_node_t *start = next; /**< from which point to update path to root */

        if (node->right != next) {
            next->parent->left = next->right;
            if (NULL != next->right) {
                next->right->parent = next->parent;
            }
            if (NULL != next_parent) {
                *next_parent = next->parent;
                start = next->parent;
            }
            node->right->parent = next;
            next->right = node->right;
        }

        if (BST_LEFT_CHILD_NODE == node_type) {
            node->parent->left = next;
        }
        else if (BST_RIGHT_CHILD_NODE == node_type) {
            node->parent->right = next;
        }
        else if (BST_ROOT_NODE == node_type) {
            bst->root = next;
        }

        next->parent = node->parent;
        if (NULL != next_parent && NULL == *next_parent) {
            *next_parent = next->parent;
        }

        if (NULL != node->left) {
            node->left->parent = next;
        }
        next->left = node->left;
        _bst_path_update(bst, start);
    }
    else {
        if (BST_LEFT_CHILD_NODE == node_type) {
            node->parent->left = node->left;
        }
        else if (BST_RIGHT_CHILD_NODE == node_type) {
            node->parent->right = node->left;
        }
        else if (BST_ROOT_NODE == node_type) {
            bst->root = node->left;
        }
        
        if (NULL != node->left) {
            node->left->parent = node->parent;
        }
        if (NULL != next_parent) {
            *next_parent = node->parent;
        }
        _bst_path_update(bst, node->parent);
    }

    if (NULL != node) {
        node->parent = NULL;
        node->left = NULL;
        node->right = NULL;
    }
}

static bst_node_t *
_bst_remove_common(bst_t *bst, void *key, bst_node_t **next_parent)
{
    bst_node_t *node = NULL;

    node = _bst_find(bst, key);

    if (NULL == node) {
        return NULL;
    }

    _bst_remove_node(bst, node, next_parent);

    return node;
}

static bst_node_t *
_bst_remove(bst_t *bst, void *key)
{
    bst_node_t *node = _bst_remove_common(bst, key, NULL);

    return node;
}

bst_node_t *
bst_remove(bst_t *bst, void *key)
{
    return bst->remove(bst, key);
}

void
bst_range(bst_t *bst, void *key_low, void *key_high, bst_traverse_t fn, void *arg)
{
    bst_node_t *node;
    for (node = bst_parent_find(bst, key_low);
         NULL != node && bst->cmp(key_high, node->data) >= 0;
         node = bst_next(bst, node)) {
        if (bst->cmp(key_low, node->data) <= 0) {
            fn(node->data, arg);
        }
    }
}

static bst_node_t *
_bst_node_kth_order_statistic(bst_node_t *node, int k)
{
    int size_left = 0;

    if (NULL == node) {
        return NULL;
    }

    if (k > node->size) {
        return NULL;
    }

    if (NULL != node->left) {
        size_left = node->left->size;
    }

    if ( k <= size_left) {
        return _bst_node_kth_order_statistic(node->left, k);
    }

    k = k - size_left - 1;

    if (k == 0) {
        return node;
    }

    return _bst_node_kth_order_statistic(node->right, k);
}

bst_node_t *
bst_kth_order_statistic(bst_t *bst, int k)
{
    bst_node_t *node = bst->root;
    return _bst_node_kth_order_statistic(node, k);
}

static void
_bst_node_rotate_right(bst_t *bst, bst_node_t *node)
{
    bst_node_type_t node_type = bst_node_type_get(node);
    bst_node_t *left = node->left;

    if (NULL != left) {
        bst_node_t *parent = node->parent;
        bst_node_t *right  = left->right;

        if (BST_LEFT_CHILD_NODE == node_type) {
            parent->left = left;
        }
        else if (BST_RIGHT_CHILD_NODE == node_type) {
            parent->right = left;
        }
        else if (BST_ROOT_NODE == node_type) {
            bst->root = left;
        }
        left->parent = parent;
        left->right = node;
        node->parent = left;
        node->left = right;
        if (NULL != right) {
            right->parent = node;
        }
    }
}

static void
_bst_node_rotate_left(bst_t *bst, bst_node_t *node)
{
    bst_node_type_t node_type = bst_node_type_get(node);
    bst_node_t *right = node->right;

    if (NULL != right) {
        bst_node_t *parent = node->parent;
        bst_node_t *left  = right->left;

        if (BST_LEFT_CHILD_NODE == node_type) {
            parent->left = right;
        }
        else if (BST_RIGHT_CHILD_NODE == node_type) {
            parent->right = right;
        }
        else if (BST_ROOT_NODE == node_type) {
            bst->root = right;
        }
        right->parent = parent;
        right->left = node;
        node->parent = right;
        node->right = left;
        if (NULL != left) {
            left->parent = node;
        }
    }
}

static void
_bst_node_rebalance_right(bst_t *bst, bst_node_t *node)
{
    if (NULL != node->left) {
        bst_node_t *left  = node->left->left;
        bst_node_t *right = node->left->right;
        if (bst_node_height(bst->type, right) > bst_node_height(bst->type, left)) {
            bst_node_t *node_left = node->left;
            _bst_node_rotate_left(bst, node->left);
            _bst_path_update(bst, node_left);
        }
    }

    _bst_node_rotate_right(bst, node);
    _bst_path_update(bst, node);
}

static void
_bst_node_rebalance(bst_t *bst, bst_node_t *node)
{
    if (NULL == node) {
        return;
    }

    bst_node_t *left  = node->left;
    bst_node_t *right = node->right;

    int diff = bst_node_height(bst->type, left) - bst_node_height(bst->type, right);

    if (diff > 1 || diff < -1) {
        if (bst_node_height(bst->type, right) > bst_node_height(bst->type, left)) {
            _bst_node_rotate_left(bst, node);
            _bst_path_update(bst, node);
        }
        else {
            _bst_node_rebalance_right(bst, node);
        }
    }
    _bst_node_rebalance(bst, node->parent);
}

static bst_node_t *
_bst_avl_insert(bst_t *bst, void *data, void *key)
{
    bst_node_t *node = _bst_insert(bst, data, key);
    if (NULL != node) {
        _bst_node_rebalance(bst, node);
    }
    return node;
}

bst_node_t *
bst_insert(bst_t *bst, void *data, void *key)
{
    return bst->insert(bst, data, key);
}

static bst_node_t *
_bst_avl_remove(bst_t *bst, void *key)
{
    bst_node_t *next_parent = NULL;
    bst_node_t *node = _bst_remove_common(bst, key, &next_parent);
    if (NULL != node) {
        _bst_node_rebalance(bst, next_parent);
    }

    return node;
}

static void 
_bst_merge_with_root(bst_t *bst, bst_t *bst_left, bst_t *bst_right)
{
    bst_node_t *left = bst_left->root;
    bst_node_t *right = bst_right->root;

    if (NULL != left) {
        left->parent = bst->root;
    }

    if (NULL != right) {
        right->parent = bst->root;
    }

    bst->root->left  = left;
    bst->root->right = right;

    _bst_path_update(bst, bst->root);

    bst_left->root = NULL;
    bst_right->root = NULL;
}

typedef void (*bst_merge_with_root_t)(bst_t *, bst_t *, bst_t *);

static void 
_bst_merge_common(bst_t *bst, bst_t *left, bst_t *right, bst_merge_with_root_t merge)
{
    bst_node_t *last = bst_last(left);

    if (NULL == last) {
        bst->root = right->root;
        right->root = NULL;
        return;
    }

    _bst_remove_node(left, last, NULL);

    bst->root = last;

    merge(bst, left, right);
}

static void
_bst_merge(bst_t *bst, bst_t *left, bst_t *right)
{
    _bst_merge_common(bst, left, right, _bst_merge_with_root);
}

static void 
_bst_avl_merge_with_root(bst_t *bst, bst_t *bst_left, bst_t *bst_right)
{
    int diff = bst_node_height(bst->type, bst_left->root) - bst_node_height(bst->type, bst_right->root);

    if (diff >= -1 && diff <= 1) {
        _bst_merge_with_root(bst, bst_left, bst_right);
        return;
    }

    if (bst_node_height(bst->type, bst_left->root) < bst_node_height(bst->type, bst_right->root)) {
        bst_node_t *root = bst_right->root;
        bst_right->root = root->left;
        _bst_avl_merge_with_root(bst, bst_left, bst_right);
        root->left = bst->root;
        bst->root->parent = root;
        bst->root = root;
        root->parent = NULL;
        _bst_path_update(bst, bst->root);
        _bst_node_rebalance(bst, bst->root);
        return;
    }

    bst_node_t *root = bst_left->root;
    bst_left->root = root->right;
    _bst_avl_merge_with_root(bst, bst_left, bst_right);
    root->right = bst->root;
    bst->root->parent = root;
    bst->root = root;
    root->parent = NULL;
    _bst_path_update(bst, bst->root);
    _bst_node_rebalance(bst, bst->root);
}

static void
_bst_avl_merge(bst_t *bst, bst_t *left, bst_t *right)
{
    _bst_merge_common(bst, left, right, _bst_avl_merge_with_root);
}

void
bst_merge(bst_t *bst, bst_t *left, bst_t *right)
{
    bst->merge(bst, left, right);
}

static void
_bst_split_common(bst_t *bst, bst_t *bst_left, bst_t *bst_right, void *key, bst_merge_with_root_t merge_with_root)
{
    if (NULL == bst->root) {
        bst_left->root = NULL;
        bst_right->root = NULL;
        return;
    }

    if (bst->cmp(key, bst->root->data) < 0) {
        if (NULL != bst->root->left) {
            bst->root->left->parent = NULL;
        }
        bst_node_t *root = bst->root;
        bst->root = bst->root->left;
        _bst_split_common(bst, bst_left, bst_right, key, merge_with_root);
        bst->root = root;
        bst_t bst_save_right = *bst;
        bst_save_right.root = root->right;
        if (NULL != bst->root->right) {
            bst->root->right->parent = NULL;
        }
        merge_with_root(bst, bst_right, &bst_save_right);
        bst_right->root = bst->root;
        bst->root = NULL;
        return;
    }

    if (NULL != bst->root->right) {
        bst->root->right->parent = NULL;
    }

    bst_node_t *root = bst->root;
    bst->root = bst->root->right;
    _bst_split_common(bst, bst_left, bst_right, key, merge_with_root);
    bst->root = root;
    bst_t bst_save_left = *bst;
    bst_save_left.root = root->left;
    if (NULL != bst->root->left) {
        bst->root->left->parent = NULL;
    }

    merge_with_root(bst, &bst_save_left, bst_left);
    bst_left->root = bst->root;
    bst->root = NULL;
}

static void
_bst_split(bst_t *bst, bst_t *left, bst_t *right, void *key)
{
    _bst_split_common(bst, left, right, key, _bst_merge_with_root); 
}

static void
_bst_avl_split(bst_t *bst, bst_t *left, bst_t *right, void *key)
{
    _bst_split_common(bst, left, right, key, _bst_avl_merge_with_root); 
}

void
bst_split(bst_t *bst, bst_t *left, bst_t *right, void *key)
{
    bst->split(bst, left, right, key);
}

static void
_bst_splay_rotate_right(bst_node_t *node, bst_node_t *parent)
{
    parent->left = node->right;
    if (NULL != node->right) {
        node->right->parent = parent;
    }
    node->right = parent;
    parent->parent = node;
}

static void
_bst_splay_rotate_left(bst_node_t *node, bst_node_t *parent)
{
    parent->right = node->left;
    if (NULL != node->left) {
        node->left->parent = parent;
    }
    node->left = parent;
    parent->parent = node;
}

static void
_bst_splay(bst_t *bst, bst_node_t *node)
{
    bst_node_type_t type = bst_node_type_get(node);

    if (BST_ROOT_NODE == type || BST_NONE_NODE == type) {
        return;
    }
    
    bst_node_t *parent = node->parent;
    bst_node_type_t parent_type = bst_node_type_get(parent);

    /* 1. zig case */
    if (BST_ROOT_NODE == parent_type) {

        bst->root = node;
        node->parent = parent->parent;

        if (BST_LEFT_CHILD_NODE == type) {
            _bst_splay_rotate_right(node, parent);
        }
        else { /* BST_RIGHT_CHILD_NODE == type */
            _bst_splay_rotate_left(node, parent);
        }
        _bst_path_update(bst, node);
        return;
    }

    bst_node_t *grandpa = parent->parent;
    bst_node_type_t grandpa_type = bst_node_type_get(grandpa);

    if (BST_LEFT_CHILD_NODE == grandpa_type) {
        grandpa->parent->left = node;
    }
    else if (BST_RIGHT_CHILD_NODE == grandpa_type) {
        grandpa->parent->right = node;
    }
    else { /* BST_ROOT_NODE == grandpa_type */
        bst->root = node;
    }
    node->parent = grandpa->parent;

    /* 2. zig-zig case */
    if (type == parent_type) {

        if (BST_LEFT_CHILD_NODE == parent_type) {
            _bst_splay_rotate_right(node, parent);
            _bst_splay_rotate_right(parent, grandpa);
        }
        else { /* BST_RIGHT_CHILD_NODE == parent_type */
            _bst_splay_rotate_left(node, parent);
            _bst_splay_rotate_left(parent, grandpa);
        }
        _bst_path_update(bst, grandpa);
    }
    else {
    
        /* 3. zig-zag case */

        if (BST_LEFT_CHILD_NODE == type) {
            _bst_splay_rotate_right(node, parent);
            _bst_splay_rotate_left(node, grandpa);
        }
        else { /* BST_RIGHT_CHILD_NODE == type */
            _bst_splay_rotate_left(node, parent);
            _bst_splay_rotate_right(node, grandpa);
        }
        _bst_path_update(bst, grandpa);
        _bst_path_update(bst, parent);
    }
    _bst_splay(bst, node);
}

static bst_node_t *
_bst_splay_find(bst_t *bst, void *key)
{
    bst_node_t *node = bst_parent_find(bst, key);

    if (NULL != node) {
        int ret = bst->cmp(key, node->data);
        _bst_splay(bst, node);
        if (ret) {
            node = NULL;
        }
    }
    return node;
}

static bst_node_t *
_bst_splay_insert(bst_t *bst, void *data, void *key)
{
    bst_node_t *node = _bst_insert(bst, data, key);
    (void)_bst_splay(bst, node);
    return node;
}

static bst_node_t *
_bst_splay_remove(bst_t *bst, void *key)
{
    bst_node_t *node = _bst_find(bst, key);
    if (NULL != node) {
        bst_node_t *max = NULL;
        _bst_splay(bst, node);
        bst_t bst_left = *bst;
        bst_t bst_right = *bst;
        bst_left.root = node->left;
        bst_right.root = node->right;
        if (NULL != node->left) {
            node->left->parent = NULL;
        }
        if (NULL != node->right) {
            node->right->parent = NULL;
        }
        node->left = NULL;
        node->right = NULL;
        max = bst_last(&bst_left);
        _bst_splay(&bst_left, max);
        bst->root = max;
        max->right = bst_right.root;
        if (NULL != max->right) {
            max->right->parent = max;
        }
        _bst_path_update(bst, max);
    }
    return node;
}

static void
_bst_splay_split(bst_t *bst, bst_t *bst_left, bst_t *bst_right, void *key)
{
    bst_node_t *node = bst_parent_find(bst, key);
    if (NULL != node) {
        _bst_splay(bst, node);
        if (bst->cmp(key, node->data) <= 0) {
            bst_left->root = node->left;
            if (NULL != node->left) {
                node->left->parent = NULL;
            }
            node->left = NULL;
            bst_right->root = node;
            _bst_path_update(bst_right, bst_right->root);
        }
        else {
            bst_right->root = node->right;
            if (NULL != node->right) {
                node->right->parent = NULL;
            }
            node->left = NULL;
            bst_left->root = node;
            _bst_path_update(bst_left, bst_left->root);
        }
        bst->root = NULL;
    }
}

static void
_bst_splay_merge(bst_t *bst, bst_t *bst_left, bst_t *bst_right)
{
    bst_node_t *root = bst_last(bst_left);
    if (NULL != root) {
        _bst_splay(bst_left, root);
        root->right = bst_right->root;
        if (NULL != root->right) {
            root->right->parent = root;
        }
        bst->root = root;
        _bst_path_update(bst, root);
        bst_left->root = NULL;
        bst_right->root = NULL;
        return;
    }
    bst->root = bst_right->root;
    bst_right->root = NULL;
}

bst_t *
bst_new(bst_type_t type, bst_cmp_t cmp, bst_free_t free)
{
    bst_t *bst = calloc(1, sizeof(bst_t));
    if (NULL != bst) {
        bst->cmp = cmp;
        bst->free = free;
        bst->type = type;
        bst->insert = _bst_insert;
        bst->remove = _bst_remove;
        bst->find = _bst_find;
        bst->split = _bst_split;
        bst->merge = _bst_merge;
        if (BST_T_AVL == type) {
            bst->insert = _bst_avl_insert;
            bst->remove = _bst_avl_remove;
            bst->split = _bst_avl_split;
            bst->merge = _bst_avl_merge;
        }
        else if (BST_T_SPLAY == type) {
            bst->insert = _bst_splay_insert;
            bst->remove = _bst_splay_remove;
            bst->find = _bst_splay_find;
            bst->split = _bst_splay_split;
            bst->merge = _bst_splay_merge;
        }
    }
    return bst;
}

void
bst_free(bst_t *bst)
{
    if (NULL != bst) {
        _bst_node_recursive_free(bst, bst->root);
        bst->root = NULL;
        bst->cmp = NULL;
        bst->free = NULL;
        free(bst);
    }
}
