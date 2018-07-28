#ifndef _CARTESIAN_TREE_H_
#define _CARTESIAN_TREE_H_

typedef enum cartesian_tree_dir cartesian_tree_dir_t;

enum cartesian_tree_dir {
    CARTTREE_DIR_PREORDER,
    CARTTREE_DIR_INORDER,
    CARTTREE_DIR_POSTORDER
};

typedef int (*cartesian_tree_cmp_t)(void *, void *);
typedef void (*cartesian_tree_traverse_t)(void *, void *);

typedef struct cartesian_tree cartesian_tree_t;

cartesian_tree_t *
cartesian_tree_new(cartesian_tree_cmp_t cmp);

void
cartesian_tree_free(cartesian_tree_t *tree);

int
cartesian_tree_add(cartesian_tree_t *tree, void *data);

void
cartesian_tree_traverse(cartesian_tree_t *tree, cartesian_tree_dir_t dir, cartesian_tree_traverse_t traverse, void *arg);

#endif /* _CARTESIAN_TREE_H_ */
