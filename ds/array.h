#ifndef _ARRAY__H_
#define _ARRAY__H_

#include <stddef.h>

/**
 * Dynamic array implementation.
 * 
 * It grows and shrinks automatically based on expansion and shrinkage factor.
 * Tries to balance tradeoff between runtime and space.
 *
 * It might store pointer to objects or the objects themselves.
 *
 * Amortized cost for insertions (array_push_back) /deletions (array_pop_back) is O(1):
 *
 * Consider initial array size 1, and subsequent array insertion
 * operations having array expansions by factor (f > 1) whenever
 * array is full:
 *
 * size: 1, 2, ..., (f), (f) + 1, (f) + 2, ..., (f)^2, (f)^2 + 1, ..., (f)^i, (f)^i + 1, ...
 * 
 * The cost ci for i-th insertion operation is (1 <= i <= n):
 *
 * ci = 1 + 
 *           (i - 1), if (i - 1) is a power of (f): i - 1 = f^k => k = logf (i - 1), logf = log on base f
 *
 *           0, otherwise
 *
 * Amortized cost for insertions is: sum(ci)/n
 *
 * sum(ci) = sum (1) + , for 1 <= i <= n
 *           sum(f^k), for  1 <= k <= floor(logf(n-1))
 *
 *         = n + sum((f)^k), for 1 <= k <= floor(logf(n-1)))
 *
 * where floor(f) is greatest integer less than or equal f.
 *
 * sum(ci) =  n + (f) * ((f)^floor(logf(n-1)) - 1) / (f - 1)  
 *         <= n + (f) * ((n - 1) - 1) / (f - 1)
 *         =  n + (f) * (n - 2) / (f - 1)
 *
 * Then sum(ci) / n <= [ n + (f) * (n - 2) / (f - 1) ] / n
 *                  =  1 + [(f)/(f - 1)] * [(n - 2) / n]
 *                  =  O(1). 
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


#define array_foreach(a, c) \
    for ((c) = array_top((a)); (c); (c) = array_next((a), (c)))

#define array_rforeach(a, c) \
    for ((c) = array_back((a)); (c); (c) = array_previous((a), (c)))

#define array_foreach_safe(a, c, n) \
    for ((c) = array_top((a)); (c) && ((n) = array_next((a), (c)), 1); (c) = (n))

#define array_rforeach_safe(a, c, n) \
    for ((c) = array_back((a)); (c) && ((n) = array_previous((a), (c)), 1); (c) = (n))

typedef struct array array_t;

/**
 * @brief Create new dynamic array object.
 *
 * Items might be pointers to item objects or
 * item objects themselves. Size of each item
 * should be specified accordingly.
 *
 * @param capacity total current capacity
 * @param item_size size of each array item
 * @return new array object
 */
array_t *
array_new(size_t capacity, size_t item_size);

/**
 * @brief Crate new dynamic array object from existing
 * array of objects of size item_size.
 *
 * @param a pre-existing array of objects
 * @param n size of pre-existing array
 * @param item_size size of each item
 * @return new array object
 */
array_t *
array_build(void *a, size_t n, size_t item_size);

/**
 * @brief Release array object
 */
void
array_free(array_t *a);

/**
 * @brief Push object in the back of the array
 * 
 * Takes O(1) amortized time.
 */
int
array_push_back(array_t *a, void *data);

/**
 * @brief Remove object from the back of the array.
 *
 * Takes O(1) amortized time.
 */
void *
array_pop_back(array_t *a, void *data);

/**
 * @brief Insert object at certain array position.
 *
 * If position is more than one past the end of the
 * array, function fails.
 * If position is one past the end, it works as
 * @array_push_back.
 * Otherwise make room necessary room to insert object
 * at postion index, potentially reallocating memory
 * for array.
 *
 * Takes O(n) runtime in the worst case.
 *
 * @param a array object
 * @param data pointer to object to be inserted
 * @param index position at which to insert object
 * @return zero if success. Otherwise, -1.
 */
int
array_insert(array_t *a, void *data, int index);

/**
 * @brief Remove object at certain array position.
 *
 * Potentially reallocates memory for the array, in
 * case it falls below shrinkage factor threshold.
 * 
 * Takes O(n) runtime in the worst case.
 *
 * @param a array object
 * @param data pointer to storage where to store
 *        object to be removed. If NULL object
 *        removed is not returned
 * @param index position at which to remove object
 * @return data pointer passed as a argument
 */ 
void *
array_remove(array_t *a, void *data, int index);

/**
 * @brief Return pointer to object stored at position index.
 *
 * Takes O(1) runtime.
 */
void *
array_get(array_t *a, int index);

/**
 * @brief Return size of the array
 */
size_t
array_size(array_t *a);

/**
 * @brief Return size of each item stored in the array
 */
size_t
array_item_size(array_t *a);

/**
 * @brief Return current capacity of the array
 */
size_t
array_capacity(array_t *a);

/**
 * @brief Return pointer to array's head 
 */
void *
array_top(array_t *a);

/**
 * @brief Return pointer to object at array's tail
 */
void *
array_back(array_t *a);

/**
 * @brief Return pointer to the next object (after current)
 */
void *
array_next(array_t *a, void *current);

/**
 * @brief Return pointer to the previous object (before current)
 */
void *
array_previous(array_t *a, void *current);

/**
 * @brief Swap two elements at positions i and j
 */
void
array_swap(array_t *a, int i, int j);

/**
 * @brief Decrement array size.
 *
 * Warning: This function will make array lose track
 * of back element each time this functino is called.
 * It is useful when artificially manipulating array
 * to prevent it from rezing (see heap implementation
 * for an example).
 */
void
array_size_dec(array_t *a);

/**
 * @brief Detach internal array from array_t.
 *
 * Warning: This function will make array lose track
 * of internal array of objects.
 * It is useful when artificially manipulating array
 * to prevent it from rezing (see heap implementation
 * for an example).
 *
 * @param a array object
 * @return detached internal array.
 */
void *
array_detach(array_t *a);

/**
 * @brief Copy count objects from position index to buffer data
 *
 * @param a array object
 * @param data buffer to store copied objects
 * @param index position from which objects should be copied
 * @param count number of objects to be copied
 * @return data pointer passed as a argument.
 */
void *
array_copy(array_t *a, void *data, int index, int count);

#endif /* _ARRAY__H_ */
