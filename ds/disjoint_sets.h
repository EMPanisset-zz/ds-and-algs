#ifndef _DISJOINT_SETS__H_
#define _DISJOINT_SETS__H_

typedef struct disjoint_sets disjoint_sets_t;

disjoint_sets_t *
disjoint_sets_new(int n);

void
disjoint_sets_free(disjoint_sets_t *ds);

void
disjoint_sets_make_set(disjoint_sets_t *ds);

int
disjoint_sets_find(disjoint_sets_t *ds, int i);

void
disjoint_sets_union(disjoint_sets_t *ds, int i, int j);

#endif /* _DISJOINT_SETS__H_ */
