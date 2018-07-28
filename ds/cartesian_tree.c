#include "cartesian_tree.h"
#include "includes.h"

typedef struct carttree_node carttree_node_t;

struct carttree_node {
    carttree_node_t *left;
    carttree_node_t *right;
    carttree_node_t *parent;
    void *data;
};

static carttree_node_t *
carttree_node_new(void *data)
{
    carttree_node_t *node = calloc(1, sizeof(*node));
    if (NULL != node) {
        node->data = data;
    }
    return node;
}

static void
carttree_node_free(carttree_node_t *node)
{
    free(node);
}

struct cartesian_tree {
    carttree_node_t *root;
    carttree_node_t *last;
    cartesian_tree_cmp_t cmp;
};

cartesian_tree_t *
cartesian_tree_new(cartesian_tree_cmp_t cmp)
{
    cartesian_tree_t *tree = calloc(1, sizeof(*tree));

    if (NULL != tree) {
        tree->cmp = cmp;
    }

    return tree;
}

static void
cartesian_tree_node_free(carttree_node_t *node)
{
    if (NULL != node) {
        cartesian_tree_node_free(node->left);
        cartesian_tree_node_free(node->right);
        carttree_node_free(node);
    }
}

void
cartesian_tree_free(cartesian_tree_t *tree)
{
    if (NULL != tree) {
        cartesian_tree_node_free(tree->root);
        tree->root = NULL;
        free(tree);
    }
}

int
cartesian_tree_add(cartesian_tree_t *tree, void *data)
{
    if (NULL == tree->root) {
        tree->root = carttree_node_new(data);
        if (NULL == tree->root) {
            return -1;
        }
        tree->last = tree->root;
        return 0;
    }

    carttree_node_t *next = tree->last;

    int ret = tree->cmp(data, next->data);

    while (ret < 0 && NULL != next->parent) {
        next = next->parent;
        ret = tree->cmp(data, next->data);
    }

    carttree_node_t *node = carttree_node_new(data);
    if (NULL == node) {
        return -1;
    }
        
    if (ret < 0) {
        node->left = next;
        if (NULL != next->parent) {
            next->parent->right = node;
        }
        next->parent = node;
        if (tree->root == next) {
            tree->root = node;
        }
        tree->last = node;
        return 0;
    }

    node->parent = next;
    node->left = next->right;
    if (NULL != next->right) {
        next->right->parent = node;
    }
    next->right = node;
    tree->last = node;
    return 0;
}

static void
cartesian_tree_node_traverse(carttree_node_t *node, cartesian_tree_dir_t dir, cartesian_tree_traverse_t traverse, void *arg)
{
    if (NULL == node) {
        return;
    }
    if (CARTTREE_DIR_PREORDER == dir) {
        traverse(node->data, arg);
    }
    cartesian_tree_node_traverse(node->left, dir, traverse, arg);
    if (CARTTREE_DIR_INORDER == dir) {
        traverse(node->data, arg);
    }
    cartesian_tree_node_traverse(node->right, dir, traverse, arg);
    if (CARTTREE_DIR_POSTORDER == dir) {
        traverse(node->data, arg);
    }
}

void
cartesian_tree_traverse(cartesian_tree_t *tree, cartesian_tree_dir_t dir, cartesian_tree_traverse_t traverse, void *arg)
{
    cartesian_tree_node_traverse(tree->root, dir, traverse, arg);
}

int
cmp_int(void *v1, void *v2)
{
    int i1 = *(int *)v1;
    int i2 = *(int *)v2;

    if (i1 < i2) {
        return -1;
    }

    if (i1 > i2) {
        return 1;
    }

    return 0;
}

void
print_int(void * v, void *arg)
{
    int i = *(int *)v;

    fprintf(stdout, "%d ", i);
}

int
main(int argc, char **argv)
{
    int a[] = { 9, 2, 8, 10, 1, 5, 6, 7, 2, 10, 3 };
    int i;
    cartesian_tree_t *tree = cartesian_tree_new(cmp_int);

    for (i = 0; i < countof(a); ++i) {
        cartesian_tree_add(tree, &a[i]);
    }

    cartesian_tree_traverse(tree, CARTTREE_DIR_INORDER, print_int, NULL);

    fprintf(stdout, "\n");

    cartesian_tree_free(tree); 

    return 0;
}
