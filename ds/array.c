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

/**
 * Dynamic array.
 *
 * One might store pointer to objects or the objects themselves
 * in the array.
 *
 * Amortized cost for insertions/deletions is O(1):
 *
 * Consider initial array size 1, and subsequent array insertion
 * operations having array expansions by factor (f) whenever array
 * is full:
 *
 * 1, 2, ..., (f), (f) + 1, (f) + 2, ..., (f)^2, (f)^2 + 1, ..., (f)^i, (f)^i + 1, ...  
 *    ^             ^                             ^               ^
 *    |             |                             |               | 
 *
 *  floor(f)    floor((f)^2)                  floor((f)^3)  floor((f)^(i + 1))
 *
 * where floor(f) is greatest integer less than or equal f.
 * 
 * The cost ci for i-th insertion operation is:
 *
 * ci = 1 + 
 *           (i - 1), if (i - 1) is a power of (f)
 *
 *           0, otherwise
 *
 * Amortized cost for insertions is: sum(ci)/n, for 1 < i < n
 *
 * sum(ci) = n + sum((f)^j), for 1 < j < floor(log(n-1)/log(f))
 *
 * sum(ci) = n + (f) * ((f)^floor(log(n-1)/log(f)) - 1) / (f - 1)  
 *         = n + (f) * ((n - 1) - 1) / (f - 1)
 *         = n + (f) * (n - 2) / (f - 1)
 *
 * Then sum(ci) / n = [ n + (f) * (n - 2) / (f - 1) ] / n
 *                  =   1 + [(f)/(f - 1)] * [(n - 2) / n]
 *                  = O(1). 
 *
 * Growth factor of 1.5 prevents wasting too much space
 * and rapidly run out of memory when expanding array.
 *
 * Shrinking the array when removing elements prevents
 * wasting too much space for unused slots.
 *
 * Different factors for growth and shrinkage protects 
 * against bad cases of consecutive alternate
 * insertions/removals which would degrade amortized
 * runtime.
 */

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

    a->array = malloc(capacity * item_size);
    if (NULL == a->array) {
        free(a);
        return NULL;
    }
    a->capacity = capacity;
    a->size = 0;
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

void *
array_top(array_t *a)
{
    if (a->size == 0) {
        return NULL;
    }

    return array_item_get(a, 0);
    
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
             * Interesting fact:
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
             * Let's take the minimum possible i, i = ceil(-log(f - 1)/log(f)).
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

/**
 * Insert new element at the end of the array.
 */
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
    memcpy(data, array_item_get(a, a->size), a->item_size);
    array_shrink(a);
    return data;
}

size_t
array_size(array_t *a)
{
    return a->size;
}

size_t
array_capacity(array_t *a)
{
    return a->capacity;
}

void *
array_first(array_t *a)
{
    return a->array;
}

void *
array_next(array_t *a, void *next)
{
    char *p = next;
    char *end = ((char *)a->array) + a->size * a->item_size;
    if (p + a->item_size < end) {
        return p + a->item_size;
    }
    return NULL;
}
