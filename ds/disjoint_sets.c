#include "disjoint_sets.h"
#include <stdlib.h>

typedef struct set set_t;

struct set {
    int parent;
    int rank;
    int size;
    void *data;
};

struct disjoint_sets {
    int n;
    set_t *sets;
};

disjoint_sets_t *
disjoint_sets_new(int n)
{
    disjoint_sets_t *ds = calloc(1, sizeof(disjoint_sets_t));
    if (NULL != ds) {
        ds->sets = calloc(n, sizeof(set_t));
        if (NULL == ds->sets) {
            goto error;
        }
        ds->n = n;
    }
    return ds;

error:
    disjoint_sets_free(ds);
    return NULL;
}

void
disjoint_sets_free(disjoint_sets_t *ds)
{
    if (NULL != ds) {
        free(ds->sets);
        ds->sets = NULL;
        free(ds);
    }
}

void
disjoint_sets_make_set(disjoint_sets_t *ds)
{
    int i;
    for (i = 0; i < ds->n; ++i) {
        ds->sets[i].parent = i;
        ds->sets[i].size = 1;
        ds->sets[i].rank = 0;
    }
}

int
disjoint_sets_find(disjoint_sets_t *ds, int i)
{
    while (i != ds->sets[i].parent) {
        ds->sets[i].parent = ds->sets[ds->sets[i].parent].parent;
        i = ds->sets[i].parent;
    }
    return i;
}

void
disjoint_sets_union(disjoint_sets_t *ds, int i, int j)
{
    int id_i = disjoint_sets_find(ds, i);
    int id_j = disjoint_sets_find(ds, j);

    if (id_i == id_j) {
        return;
    }

    int parent = id_i;
    int child  = id_j;

    if (ds->sets[parent].rank < ds->sets[child].rank) {
        parent = id_j;
        child  = id_i;
    }

    ds->sets[child].parent = parent;
    ds->sets[parent].size += ds->sets[child].size;
    if (ds->sets[child].rank == ds->sets[parent].rank) {
        ds->sets[parent].rank++;
    }
}
