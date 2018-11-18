#include "array.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define array_max_value(a, b)    ((a) > (b) ? (a) : (b))

struct array {
    void *array;
    size_t capacity;
    size_t size;
    size_t item_size;
};

#define ARRAY_MAX_CAPACITY ((size_t)-1)
#define ARRAY_GROWTH_FACTOR (1.5)
#define ARRAY_SHRINKAGE_FACTOR (0.5)

#define array_item_get(a, i) (&((char *)(a)->array)[(a)->item_size * (i)])

array_t *
array_new(size_t capacity, size_t item_size)
{
    array_t *a = calloc(1, sizeof(array_t));

    if (NULL == a) {
        return NULL;
    }

    if (capacity > 0) {
        a->array = malloc(capacity * item_size);
        if (NULL == a->array) {
            free(a);
            return NULL;
        }
    }
    a->capacity = capacity;
    a->size = 0;
    a->item_size = item_size;

    return a;
}

array_t *
array_build(void *b, size_t n, size_t item_size)
{
    array_t *a = calloc(1, sizeof(array_t));
    if (NULL == a) {
        return NULL;
    }

    a->array = b;
    a->capacity = n;
    a->size = n;
    a->item_size = item_size;

    return a;
}

void
array_free(array_t *a)
{
    if (NULL != a) {
        free(a->array);
        a->array = NULL;
        free(a);
    }
}

static int
array_expand(array_t *a)
{
    if (a->size == a->capacity) {
        void *array = NULL;
        size_t capacity = 0;
        if (a->capacity == ARRAY_MAX_CAPACITY) {
            return -1;
        }
        if (a->capacity > floor(ARRAY_MAX_CAPACITY / ARRAY_GROWTH_FACTOR)) {
            capacity = ARRAY_MAX_CAPACITY;
        }
        else {
            /**
             * Reason for this adjustment:
             * 
             * Consider growth factor f, 1 < f < 2, and i-th capacity expansion.
             *
             * In order to have actual growth we need floor((f)^(i+1)) > floor((f)^i),
             * then (f)^(i+1) - (f)^i >= 1 (sufficient condition).
             *
             * That happens iff: (f)^i * (f) - (f)^i >= 1
             *              iff: (f)^i * ((f) - 1) >= 1
             *              iff: i * log(f) + log(f - 1) >= 0
             *              iff: i >= -log(f - 1)/log(f)
             *
             * Let's take the minimum possible i, i = ceil(-log(f - 1)/log(f))
             *
             * where ceil(f) is the smallest integer greater or equal to f.
             *
             * For a growth factor f = 1.5: i = ceil(-log(1.5 - 1)/log(1.5))
             *                                = ceil(-log(0.5)/log(1.5))
             *                                = ceil(1.7095) = 2
             *
             * Note: if f >= 2, for any i condition holds.
             */
            capacity = array_max_value(floor(a->capacity * ARRAY_GROWTH_FACTOR), a->capacity + 1);
        }
        array = realloc(a->array, capacity * a->item_size);
        if (NULL == array) {
            return -1;
        }
        //fprintf(stdout, "expanding array %zu -> %zu\n", a->capacity, capacity);
        a->array = array;
        a->capacity = capacity;
    }
    return 0;
}

int
array_push_back(array_t *a, void *data)
{
    if (array_expand(a) < 0) {
        return -1;
    }
    a->size++;
    memcpy(array_item_get(a, a->size - 1), data, a->item_size);
    return 0;
}

static void
array_shrink(array_t *a)
{
    if (a->size <= floor(a->capacity * ARRAY_SHRINKAGE_FACTOR)) {
        void *array= NULL;
        size_t capacity = floor(a->capacity * ARRAY_SHRINKAGE_FACTOR);
        array = realloc(a->array, capacity * a->item_size);
        if (NULL != array || 0 == capacity) {
            //fprintf(stdout, "shrinking array %zu -> %zu\n", a->capacity, capacity);
            a->capacity = capacity;
            a->array = array;
        }
    }
}

void *
array_pop_back(array_t *a, void *data)
{
    if (a->size == 0) {
        return NULL;
    }
    --a->size;
    if (NULL != data) {
        memcpy(data, array_item_get(a, a->size), a->item_size);
    }
    array_shrink(a);
    return data;
}

size_t
array_size(array_t *a)
{
    return a->size;
}

size_t
array_item_size(array_t *a)
{
    return a->item_size;
}

size_t
array_capacity(array_t *a)
{
    return a->capacity;
}

int
array_insert(array_t *a, void *data, int index)
{
    if (index > a->size) {
        return -1;
    }
    if (index == a->size) {
        return array_push_back(a, data);
    }
    if (array_expand(a) < 0) {
        return -1;
    }
    memmove(array_item_get(a, index + 1), array_item_get(a, index), a->item_size * (a->size - index));
    memcpy(array_item_get(a, index), data, a->item_size);
    a->size++;
    return 0;
}

void *
array_remove(array_t *a, void *data, int index)
{
    if (index >= a->size) {
        return NULL;
    }
    if (index == a->size - 1) {
        return array_pop_back(a, data);
    }
    if (a->size == 0) {
        return NULL;
    }
    if (NULL != data) {
        memcpy(data, array_item_get(a, index), a->item_size);
    }
    memmove(array_item_get(a, index), array_item_get(a, index + 1), a->item_size * (a->size - index - 1));
    --a->size;
    array_shrink(a);
    return data;
}

void *
array_get(array_t *a, int index)
{
    return array_item_get(a, index);
}

void *
array_top(array_t *a)
{
    if (a->size == 0) {
        return NULL;
    }

    return array_item_get(a, 0);
}

void *
array_back(array_t *a)
{
    if (a->size == 0) {
        return NULL;
    }

    return array_item_get(a, a->size - 1);
}

void *
array_next(array_t *a, void *current)
{
    char *p = current;
    char *end = ((char *)a->array) + a->size * a->item_size;
    if (p + a->item_size < end) {
        return p + a->item_size;
    }
    return NULL;
}

void *
array_previous(array_t *a, void *current)
{
    char *p = current;
    char *end = ((char *)a->array) - a->item_size;
    if (p - a->item_size > end) {
        return p - a->item_size;
    }
    return NULL;
}

void
array_swap(array_t *array, int i, int j)
{
    size_t size = array_item_size(array);
    char *a = array_get(array, i);
    char *b = array_get(array, j);

    do {
        char tmp = *a;
        *a++ = *b;
        *b++ = tmp;
    } while (--size > 0);
}

void *
array_copy(array_t *a, void *data, int index, int count)
{
    if (index < 0 || count <= 0) {
        return NULL;
    }
    if (index + count > a->size) {
        return NULL;
    }

    if (NULL == data) {
        return NULL;
    }

    memcpy(data, array_get(a, index), a->item_size * count);

    return data;
}

void
array_size_dec(array_t *a)
{
    if (a->size) {
        --a->size;
    }
}

void *
array_detach(array_t *a)
{
    void *array = a->array;
    a->array = NULL;
    a->size = 0;
    a->capacity = 0;
    return array;
}
