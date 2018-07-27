#ifndef _HEAP__H_
#define _HEAP__H_

#include <stddef.h>
#include <stdbool.h>

/**
 * Returns true if first object is considered less than or equal (min-heap) or
 * greater than or equal (max-heap) second object.
 */
typedef bool (*heap_cmp_t)(const void *, const void *);
/**
 * Called whenever respective object is deleted from heap.
 *
 * Useful in case heap stores pointer to objects.
 */
typedef void (*heap_release_t)(void *);
/**
 * Called whenever respective object moves through the heap.
 *
 * Second parameter provides new index in the heap for this object.
 */
typedef void (*heap_update_t)(void *, size_t);

typedef struct heap heap_t;

/**
 * Allocates new heap object.
 *
 * This implementation allows one to store pointer to objects or
 * the objects themselves in the heap. Size of the element to be
 * stored should be provided in item_size parameter. 
 *
 * @param capacity initial capacity of heap
 * @param item_size size of one element of the array
 * @param cmp comparison function to sort objects pointed by heap array
 * @param release function used to release objects pointed by array (optional)
 * @param update function to update index of objects in the heap as they
 *        are moved through the heap array. This index should be provided
 *        to heap_update and heap_remove function. If heap_update and heap_remove
 *        will never be used, no need to provide this function.
 */
heap_t *
heap_new(size_t capacity, size_t item_size, heap_cmp_t cmp, heap_release_t release, heap_update_t update);

/**
 * Creates heap object from array of objects.
 *
 * It's an O(n) time operation.
 *
 * @param array array of objects to be sorted
 * @param n size of array
 * @param item_size size of one element of the array
 * @param cmp @see heap_new 
 * @param release @see heap_new 
 * @param update @see heap_new 
 * @return heap object or NULL in case of error.
 */
heap_t *
heap_build(void *array, size_t n, size_t item_size, heap_cmp_t cmp, heap_release_t release, heap_update_t update);

/**
 * Sorts array of objects using heap sort algorithm.
 *
 * It's an O(n * log(n)) time operation.
 *
 * @param array array of objects to be sorted
 * @param n size of array
 * @param item_size size of one element of the array
 * @param cmp comparison function to sort objects pointed by array
 * @return zero upon success. -1 otherwise.
 */
int
heap_sort(void *array, size_t n, size_t item_size, heap_cmp_t cmp);

/**
 * Notifies heap that i-th element was just updated.
 *
 * It's an O(log(n)) time operation.
 *
 * Whenever element is externally updated and that might cause
 * its order in the heap change, heap should be notified by
 * calling this function so that this element might be moved
 * through the heap to reflect potential new position on it.
 *
 * If this function is to be used user must
 * store index i and implement heap's update
 * callback.
 *
 * @param h heap objecct
 * @param i index of element just updated
 */
void
heap_update(heap_t *h, size_t i);

/**
 * Returns element on the top of the heap (maximum/minimum element)
 * 
 * It's an O(1) constant time operation.
 *
 * @param h heap object
 * @return element on the top of the heap 
 */
void *
heap_top(heap_t *h);

/**
 * Removes element from the top of the heap (current maximum/minimum element)
 *
 * It's an O(log(n)) time operation
 *
 * @param h heap object
 * @param top pointer to a buffer in which store top of the heap
 * @return on success, element on the top of the heap (point to top param).
 *         otherwise returns NULL
 */
void *
heap_pop_front(heap_t *h, void *top);

/**
 * Inserts element in the heap
 *
 * It's an O(log(n)) time operation
 *
 * @param h heap object
 * @param data pointer to object to be inserted in the heap.
 *        object is copied into the heap from this pointer.
 * @return 0 on success, -1 otherwise
 */
int
heap_insert(heap_t *h, void *data);

/**
 * Removes i-th element from the heap
 * 
 * it's an O(log(n)) time operation
 *
 * If this function is to be used user must
 * store index i and implement heap's update
 * callback.
 *
 * if heap's release callback is provided, it's
 * called for element just removed from the heap.
 *
 * @param h heap object
 * @param i index of the element to be removed
 */
void
heap_remove(heap_t *h, size_t i);

/**
 * Releases heap object.
 *
 * if heap's release callback is provided, it's
 * called for every element stored in the heap.
 *
 * @param h heap object
 */
void
heap_free(heap_t *h);

/**
 * Returns current number of elements in the heap.
 *
 * @param h heap object
 */
size_t
heap_size(heap_t *h);

/**
 * Returns current heap capacity.
 *
 * @param h heap object
 */
size_t
heap_capacity(heap_t *h);

/**
 * Returns internal heap array
 *
 * @param h heap object
 */
void *
heap_array(heap_t *h);

#endif /* _HEAP__H_ */
