#ifndef _ARRAY__H_
#define _ARRAY__H_

#include <stddef.h>

#define array_foreach(a, n) \
    for ((n) = array_first((a)); (n); (n) = array_next((a), (n)))

typedef struct array array_t;

array_t *
array_new(size_t capacity, size_t item_size);

void
array_free(array_t *a);


void *
array_top(array_t *a);

int
array_push_back(array_t *a, void *data);

void *
array_pop_back(array_t *a, void *data);

size_t
array_size(array_t *a);

size_t
array_capacity(array_t *a);

void *
array_first(array_t *a);

void *
array_next(array_t *a, void *next);

#endif /* _ARRAY__H_ */
