#include "heap.h"
#include "array.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define heap_max_value(a, b)    ((a) > (b) ? (a) : (b))

struct heap {
    array_t *array;
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
 */

#define parent(i) (((i) - 1) >> 1)
#define left_child(i) (((i) << 1) + 1)
#define right_child(i) (((i) + 1) << 1)

heap_t *
heap_new(size_t capacity, size_t item_size, heap_cmp_t cmp, heap_release_t release, heap_update_t update)
{
    heap_t *h = calloc(1, sizeof(heap_t));

    if (NULL == h) {
        return NULL;
    }

    h->array = array_new(capacity, item_size);
    if (NULL == h->array) {
        free(h);
        return NULL;
    }
    h->cmp = cmp;
    h->release = release;
    h->update = update;

    return h;
}

void
heap_free(heap_t *h)
{
    if (h->array) {
        if (h->release) {
            void *c, *n;
            array_foreach_safe(h->array, c, n) { 
                h->release(c);
            }
            h->release = NULL;
        }
        array_free(h->array);
    }
    h->cmp = NULL;
    h->update = NULL;
    h->array = NULL;
    free(h);
}

static inline void
heap_swap(heap_t *h, int i, int j)
{
    array_swap(h->array, i, j);

    if (h->update) {
        h->update(array_get(h->array, i), i);
        h->update(array_get(h->array, j), j);
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

    while (l < array_size(heap->array)) {

        winner = l;

        if (r < array_size(heap->array) && heap->cmp(array_get(heap->array, r), array_get(heap->array, winner))) {
            winner = r;
        }

        if (heap->cmp(array_get(heap->array, winner), array_get(heap->array, i))) {
            heap_swap(heap, winner, i);
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
        
        if (heap->cmp(array_get(heap->array, i), array_get(heap->array, p))) {
            heap_swap(heap, i, p);
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

    h->array = array_build(array, n, item_size);
    if (NULL == h->array) {
        free(h);
        return NULL;
    }
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
            h->update(array_get(h->array, i),  i);
            if (right_child(i) < n) {
                h->update(array_get(h->array, right_child(i)), right_child(i));
            }
            if (left_child(i) < n) {
                h->update(array_get(h->array, left_child(i)), left_child(i));
            }
        }
        sift_down(h, i);
    }

    return h;
}

int
heap_sort(void *a, size_t n, size_t item_size, heap_cmp_t cmp)
{
    heap_t *h = heap_build(a, n, item_size, cmp, NULL, NULL);

    if (NULL == h) {
        return -1;
    }

    size_t i;
    for (i = 0; i < n; ++i) {
        array_swap(h->array, 0, array_size(h->array) - 1);
        array_size_dec(h->array);
        sift_down(h, 0);
    }

    array_detach(h->array);
    heap_free(h);

    return 0;
}

void
heap_update(heap_t *h, size_t i)
{
    if (i >= array_size(h->array)) {
        return;
    }
    sift_down(h, i);
    sift_up(h, i);
}

void *
heap_top(heap_t *h)
{
    return array_top(h->array);
}

void *
heap_pop_front(heap_t *h, void *top)
{
    if (array_size(h->array) == 0) {
        return NULL;
    }

    if (array_size(h->array) == 1) {
        array_pop_back(h->array, top);
    }
    else {
        array_copy(h->array, top, 0, 1);
        array_pop_back(h->array, array_top(h->array));

        if (h->update) {
            h->update(array_top(h->array), 0);
        }

        sift_down(h, 0);
    }

    return top;
}

/**
 * Insert new element at the end of the tree
 * to preserve tree completeness.
 */
int
heap_insert(heap_t *h, void *data)
{
    array_push_back(h->array, data);
    if (h->update) {
        h->update(array_back(h->array), array_size(h->array) - 1);
    }
    sift_up(h, array_size(h->array) - 1);
    return 0;
}

/**
 * last element of the heap takes place of
 * removed element so as to preserve tree
 * completeness.
 */
void
heap_remove(heap_t *h, size_t i)
{
    if (i >= array_size(h->array)) {
        return;
    }

    if (h->release) {
        h->release(array_get(h->array, i));
    }

    if (i == array_size(h->array) - 1) {
        array_pop_back(h->array, NULL);
    }
    else {
        array_pop_back(h->array, array_get(h->array, i));

        if (h->update) {
            h->update(array_get(h->array, i), i);
        }
        heap_update(h, i);
    }
}

size_t
heap_size(heap_t *h)
{
    return array_size(h->array);
}

size_t
heap_capacity(heap_t *h)
{
    return array_capacity(h->array);
}

array_t *
heap_array(heap_t *h)
{
    return h->array;
}
