#include "heap.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define heap_max_value(a, b)    ((a) > (b) ? (a) : (b))

struct heap {
    void *array;
    size_t capacity;
    size_t size;
    size_t item_size;
    heap_cmp_t cmp;
    heap_release_t release;
    heap_update_t update;
};

/**
 * Heap stores elements in a dynamic array.
 *
 * One might store pointer to objects or the objects themselves
 * in the heap array.
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
 * insertions/removals wich would degrade amortized
 * runtime.
 */

#define HEAP_MAX_CAPACITY ((size_t)-1)
#define HEAP_GROWTH_FACTOR (1.5)
#define HEAP_SHRINKAGE_FACTOR (0.5)

#define parent(i) (((i) - 1) >> 1)
#define left_child(i) (((i) << 1) + 1)
#define right_child(i) (((i) + 1) << 1)

#define heap_item_get(h, i) (&((char *)(h)->array)[(h)->item_size * (i)])

heap_t *
heap_new(size_t capacity, size_t item_size, heap_cmp_t cmp, heap_release_t release, heap_update_t update)
{
    heap_t *h = calloc(1, sizeof(heap_t));

    if (NULL == h) {
        return NULL;
    }

    h->array = malloc(capacity * item_size);
    if (NULL == h->array) {
        free(h);
        return NULL;
    }
    h->capacity = capacity;
    h->size = 0;
    h->item_size = item_size;
    h->cmp = cmp;
    h->release = release;
    h->update = update;

    return h;
}

void
heap_free(heap_t *h)
{
    if (h->release) {
        size_t i;
        for (i = 0; i < h->size; ++i) {
            h->release(heap_item_get(h, i));
            memset(heap_item_get(h, i), 0, h->item_size);
        }
        h->release = NULL;
    }
    h->cmp = NULL;
    h->update = NULL;
    free(h->array);
    h->array = NULL;
    free(h);
}

static inline void
swap(heap_t *h, int i, int j)
{
    size_t size = h->item_size;
    char *a = heap_item_get(h, i);
    char *b = heap_item_get(h, j);

    do {
        char tmp = *a;
        *a++ = *b;
        *b++ = tmp;
    } while (--size > 0);
 
    if (h->update) {
        h->update(heap_item_get(h, i), i);
        h->update(heap_item_get(h, j), j);
    }
}

static void
sift_down(heap_t *heap, size_t i)
{
    size_t l;
    size_t r;
    size_t winner;

    l = left_child(i);
    r = right_child(i);

    while (l < heap->size) {

        winner = l;

        if (r < heap->size && heap->cmp(heap_item_get(heap, r), heap_item_get(heap, winner))) {
            winner = r;
        }

        if (heap->cmp(heap_item_get(heap, winner), heap_item_get(heap, i))) {
            swap(heap, winner, i);
        }
        else {
            winner = i;
        }

        if (winner == i) {
            break;
        }

        i = winner;
    
        l = left_child(i);
        r = right_child(i);
    }
}

static void
sift_up(heap_t *heap, size_t i)
{
    while (i > 0) {
        size_t p = parent(i);
        
        if (heap->cmp(heap_item_get(heap, i), heap_item_get(heap, p))) {
            swap(heap, i, p);
            i = p;
        }
        else {
            break;
        }
    }
}

heap_t *
heap_build(void *array, size_t n, size_t item_size, heap_cmp_t cmp, heap_release_t release, heap_update_t update)
{
    heap_t *h = calloc(1, sizeof(heap_t));

    if (NULL == h) {
        return NULL;
    }

    if (0 == n) {
        return NULL;
    }

    h->array = array;
    h->capacity = n;
    h->size = n;
    h->item_size = item_size;
    h->cmp = cmp;
    h->release = release;
    h->update = update;

    /** 
     * heapify down subtrees from bottom-up starting from level just above leaves
     * (subtrees rooted on leaves already meet heap property).
     */
    int i;
    for (i = parent(n-1); i >= 0; --i) {
        if (h->update) {
            h->update(heap_item_get(h, i),  i);
            if (right_child(i) < n) {
                h->update(heap_item_get(h, right_child(i)),right_child(i));
            }
            if (left_child(i) < n) {
                h->update(heap_item_get(h, left_child(i)), left_child(i));
            }
        }
        sift_down(h, i);
    }

    return h;
}

int
heap_sort(void *array, size_t n, size_t item_size, heap_cmp_t cmp)
{
    heap_t *h = heap_build(array, n, item_size, cmp, NULL, NULL);

    if (NULL == h) {
        return -1;
    }

    size_t i;
    void *top = malloc(item_size);
    if (NULL == top) {
        return -1;
    }
    for (i = 0; i < n; ++i) {
        if (NULL == heap_pop_front(h, top)) {
            free(top);
            return -1;
        }
        memcpy(heap_item_get(h, h->size), top, h->item_size);
    }
    free(top);

    h->array = NULL;

    heap_free(h);

    return 0;
}

void
heap_update(heap_t *h, size_t i)
{
    if (i >= h->size) {
        return;
    }
    sift_down(h, i);
    sift_up(h, i);
}

void *
heap_top(heap_t *h)
{
    if (h->size == 0) {
        return NULL;
    }

    return heap_item_get(h, 0);
    
}

void *
heap_pop_front(heap_t *h, void *top)
{
    if (h->size == 0) {
        return NULL;
    }

    memcpy(top, heap_item_get(h, 0), h->item_size);
    h->size--;

    if (h->size == 0) {
        return top;
    }

    memcpy(heap_item_get(h, 0), heap_item_get(h, h->size), h->item_size);

    if (h->update) {
        h->update(heap_item_get(h, 0), 0);
    }

    sift_down(h, 0);

    return top;
}

static int
heap_expand(heap_t *h)
{
    if (h->size == h->capacity) {
        void *array = NULL;
        size_t capacity = 0;
        if (h->capacity == HEAP_MAX_CAPACITY) {
            return -1;
        }
        if (h->capacity > floor(HEAP_MAX_CAPACITY / HEAP_GROWTH_FACTOR)) {
            capacity = HEAP_MAX_CAPACITY;
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
            capacity = heap_max_value(floor(h->capacity * HEAP_GROWTH_FACTOR), h->capacity + 1);
        }
        array = realloc(h->array, capacity * h->item_size);
        if (NULL == array) {
            return -1;
        }
        //fprintf(stdout, "expanding heap %zu -> %zu\n", h->capacity, capacity);
        h->array = array;
        h->capacity = capacity;
    }
    return 0;
}

/**
 * Insert new element at the end of the tree
 * to preserve tree completeness.
 */
int
heap_insert(heap_t *h, void *data)
{
    if (heap_expand(h) < 0) {
        return -1;
    }
    h->size++;
    memcpy(heap_item_get(h, h->size - 1), data, h->item_size);
    if (h->update) {
        h->update(heap_item_get(h, h->size -1), h->size - 1);
    }
    sift_up(h, h->size - 1);
    return 0;
}

static void
heap_shrink(heap_t *h)
{
    if (h->size <= floor(h->capacity * HEAP_SHRINKAGE_FACTOR)) {
        void *array= NULL;
        size_t capacity = floor(h->capacity * HEAP_SHRINKAGE_FACTOR);
        array = realloc(h->array, capacity * h->item_size);
        if (NULL != array || 0 == capacity) {
            //fprintf(stdout, "shrinking heap %zu -> %zu\n", h->capacity, capacity);
            h->capacity = capacity;
            h->array = array;
        }
    }
}

/**
 * last element of the heap takes place of
 * removed element so as to preserve tree
 * completeness.
 */
void
heap_remove(heap_t *h, size_t i)
{
    if (i >= h->size) {
        return;
    }
    if (h->release) {
        h->release(heap_item_get(h, i));
    }
    --h->size;
    if (i != h->size) {
        memcpy(heap_item_get(h, i), heap_item_get(h, h->size), h->item_size);
    }
    heap_shrink(h);
    if (i == h->size) {
        return;
    }
    if (h->update) {
        h->update(heap_item_get(h, i), i);
    }
    heap_update(h, i);
}

size_t
heap_size(heap_t *h)
{
    return h->size;
}

size_t
heap_capacity(heap_t *h)
{
    return h->capacity;
}

void *
heap_array(heap_t *h)
{
    return h->array;
}
