#ifndef _SUFFIX_TREE__H_
#define _SUFFIX_TREE__H_

typedef struct suffix_tree suffix_tree_t;

suffix_tree_t *
suffix_tree_new(const char* string);

void
suffix_tree_free(suffix_tree_t *tree);

#endif /* _SUFFIX_TREE__H_ */
