#ifndef _DEQUE__H_
#define _DEQUE__H_

#include <stddef.h>

#define deque_foreach(a, c) \
    for ((c) = deque_front((a)); (c); (c) = deque_next((a), (c)))

#define deque_rforeach(a, c) \
    for ((c) = deque_back((a)); (c); (c) = deque_previous((a), (c)))

typedef struct deque deque_t;

/**
 * @brief Create new dynamic deque object.
 *
 * Items might be pointers to item objects or
 * item objects themselves. Size of each item
 * should be specified accordingly.
 *
 * @param capacity total current capacity
 * @param item_size size of each deque item
 * @return new deque object
 */
deque_t *
deque_new(size_t capacity, size_t item_size);

/**
 * @brief Create new dynamic deque object from existing
 * array of objects of size item_size.
 *
 * @param a pre-existing array of objects
 * @param n size of pre-existing array 
 * @param item_size size of each item
 * @return new deque object
 */
deque_t *
deque_build(void *a, size_t n, size_t item_size);

/**
 * @brief Release deque object
 */
void
deque_free(deque_t *d);

/**
 * @brief Push object in the back of the deque
 * 
 * Takes O(1) amortized time.
 */
int
deque_push_back(deque_t *d, void *data);

int
deque_push_front(deque_t *d, void *data);

/**
 * @brief Remove object from the back of the deque.
 *
 * Takes O(1) amortized time.
 */
void *
deque_pop_back(deque_t *d, void *data);

void *
deque_pop_front(deque_t *d, void *data);

/**
 * @brief Insert object at certain deque position.
 *
 * If position is more than one past the end of the
 * deque, function fails.
 * If position is one past the end, it works as
 * @deque_push_back.
 * Otherwise make necessary room to insert object
 * at postion index, potentially reallocating memory
 * for deque.
 *
 * Takes O(n) runtime in the worst case.
 *
 * @param d deque object
 * @param data pointer to object to be inserted
 * @param index position at which to insert object
 * @return zero if success. Otherwise, -1.
 */
int
deque_insert(deque_t *d, void *data, int index);

/**
 * @brief Remove object at certain deque position.
 *
 * Potentially reallocates memory for the deque, in
 * case it falls below shrinkage factor threshold.
 * 
 * Takes O(n) runtime in the worst case.
 *
 * @param d deque object
 * @param data pointer to storage where to store
 *        object to be removed. If NULL object
 *        removed is not returned
 * @param index position at which to remove object
 * @return data pointer passed as a argument
 */ 
void *
deque_remove(deque_t *d, void *data, int index);

/**
 * @brief Return pointer to object stored at position index.
 *
 * Takes O(1) runtime.
 */
void *
deque_get(deque_t *d, int index);

/**
 * @brief Return size of the deque
 */
size_t
deque_size(deque_t *d);

/**
 * @brief Return size of each item stored in the deque
 */
size_t
deque_item_size(deque_t *d);

/**
 * @brief Return current capacity of the deque
 */
size_t
deque_capacity(deque_t *d);

/**
 * @brief Return pointer to deque's head 
 */
void *
deque_front(deque_t *d);

/**
 * @brief Return pointer to object at deque's tail
 */
void *
deque_back(deque_t *d);

/**
 * @brief Return pointer to the next object (after current)
 */
void *
deque_next(deque_t *d, void *current);

/**
 * @brief Return pointer to the previous object (before current)
 */
void *
deque_previous(deque_t *d, void *current);

/**
 * @brief Swap two elements at positions i and j
 */
void
deque_swap(deque_t *d, int i, int j);

/**
 * @brief Detach internal array from deque_t.
 *
 * Warning: This function will make deque lose track
 * of internal deque of objects.
 * It is useful when artificially manipulating deque
 * to prevent it from rezing.
 *
 * @param a deque object
 * @return detached internal deque.
 */
void *
deque_detach(deque_t *d);

/**
 * @brief Copy count objects from position index to buffer data
 *
 * @param d deque object
 * @param data buffer to store copied objects
 * @param index position from which objects should be copied
 * @param count number of objects to be copied
 * @return data pointer passed as a argument.
 */
void *
deque_copy(deque_t *d, void *data, int index, int count);

#endif /* _DEQUE__H_ */
