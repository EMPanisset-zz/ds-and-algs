#include "deque.h"
#include "includes.h"

#define deque_max_value(a, b)    ((a) > (b) ? (a) : (b))

struct deque {
    void *array;
    size_t head; /*< 1st element index */
    size_t tail; /*< one beyond last element index */
    size_t capacity;
    size_t item_size;
    bool full; /**< mark if deque reached its max capacity */
};

#define DEQUE_MAX_CAPACITY ((size_t)-1)
#define DEQUE_GROWTH_FACTOR (1.5)
#define DEQUE_SHRINKAGE_FACTOR (0.5)

#define deque_item_get(d, i) (&((char *)(d)->array)[(d)->item_size * (i)])

deque_t *
deque_new(size_t capacity, size_t item_size)
{
    deque_t *d = calloc(1, sizeof(deque_t));

    if (NULL == d) {
        return NULL;
    }

    if (capacity > 0) {
        d->array = malloc(capacity * item_size);
        if (NULL == d->array) {
            free(d);
            return NULL;
        }
    }
    d->head = 0;
    d->tail = 0;
    d->capacity = capacity;
    d->item_size = item_size;
    d->full = false;

    return d;
}

deque_t *
deque_build(void *b, size_t n, size_t item_size)
{
    deque_t *d = calloc(1, sizeof(deque_t));
    if (NULL == d) {
        return NULL;
    }

    d->array = b;
    d->head = 0;
    d->tail = 0;
    d->capacity = n;
    d->item_size = item_size;
    d->full = true;

    return d;
}

void
deque_free(deque_t *d)
{
    if (NULL != d) {
        free(d->array);
        d->array = NULL;
        free(d);
    }
}

static size_t 
deque_shift_left(deque_t *d, size_t idx, int shift)
{
    return (idx + (d->capacity - (shift % d->capacity))) % d->capacity;
}

static size_t 
deque_shift_right(deque_t *d, size_t idx, int shift)
{
    return (idx + (shift % d->capacity)) % d->capacity;
}

static bool
deque_index_valid(deque_t *d, int index)
{
    if (index < 0) {
        return false;
    }
    if (d->tail >= d->head && !d->full) {
        return index >= d->head && index < d->tail;
    }
    return (index >= d->head && index < d->capacity) ||
           (index >= 0 && index < d->tail);
}

static bool
deque_count_valid(deque_t *d, int index, int count) 
{
    if (count <= 0 || count > d->capacity) {
        return false;
    }
    if (d->tail >= index && !d->full) {
        return index + count <= d->tail;
    }
    return count <= d->capacity - index + d->tail;
}

static void * 
_deque_copy(deque_t *d, void *data, int idx, int count)
{
    if (!deque_index_valid(d, idx)) {
        return NULL;
    }

    if (!deque_count_valid(d, idx, count)) {
        return NULL;
    }

    if (idx >= d->head && idx >= d->tail) {
        size_t len = count > d->capacity - idx ? d->capacity - idx : count;
        size_t size = d->item_size * len;
        memcpy(data, deque_item_get(d, idx), size);
        if (count - len) {
            memcpy(((char *)data) + size, deque_item_get(d, 0), d->item_size * (count - len));
        }
    }
    else {
        memcpy(data, deque_item_get(d, idx), d->item_size * count);
    }
    return data;
}

static int
deque_expand(deque_t *d)
{
    size_t size = deque_size(d);
    if (size == d->capacity) {
        void *array = NULL;
        size_t capacity = 0;
        if (d->capacity == DEQUE_MAX_CAPACITY) {
            return -1;
        }
        if (d->capacity > floor(DEQUE_MAX_CAPACITY / DEQUE_GROWTH_FACTOR)) {
            capacity = DEQUE_MAX_CAPACITY;
        }
        else {
            capacity = deque_max_value(floor(d->capacity * DEQUE_GROWTH_FACTOR), d->capacity + 1);
        }
        array = malloc(capacity * d->item_size);
        if (NULL == array) {
            return -1;
        }
        _deque_copy(d, array, d->head, size);
        d->head = 0;
        d->tail = size;
        free(d->array);
        d->array = array;
        d->capacity = capacity;
        d->full = false;
    }
    return 0;
}

int
deque_push_back(deque_t *d, void *data)
{
    if (deque_expand(d) < 0) {
        return -1;
    }
    memcpy(deque_item_get(d, d->tail), data, d->item_size);
    d->tail = deque_shift_right(d, d->tail, 1);
    d->full = (d->tail == d->head);
    return 0;
}

int
deque_push_front(deque_t *d, void *data)
{
    if (deque_expand(d) < 0) {
        return -1;
    }
    d->head = deque_shift_left(d, d->head, 1);
    memcpy(deque_item_get(d, d->head), data, d->item_size);
    d->full = (d->tail == d->head);
    return 0;
}

static void
deque_shrink(deque_t *d)
{
    size_t size = deque_size(d);
    if (size <= floor(d->capacity * DEQUE_SHRINKAGE_FACTOR)) {
        void *array = NULL;
        size_t capacity = floor(d->capacity * DEQUE_SHRINKAGE_FACTOR);
        if (capacity) {
            array = malloc(capacity * d->item_size);
            if (NULL == array) {
                return;
            }
            _deque_copy(d, array, d->head, size);
        }
        d->head = 0;
        d->tail = size ? size % capacity : 0;
        free(d->array);
        d->array = array;
        d->capacity = capacity;
        d->full = (capacity && (size == capacity));
    }
}

void *
deque_pop_back(deque_t *d, void *data)
{
    if (deque_size(d) == 0) {
        return NULL;
    }
    d->tail = deque_shift_left(d, d->tail, 1);
    if (NULL != data) {
        memcpy(data, deque_item_get(d, d->tail), d->item_size);
    }
    d->full = false;
    deque_shrink(d);
    return data;
}

void *
deque_pop_front(deque_t *d, void *data)
{
    if (deque_size(d) == 0) {
        return NULL;
    }
    if (NULL != data) {
        memcpy(data, deque_item_get(d, d->head), d->item_size);
    }
    d->head = deque_shift_right(d, d->head, 1);
    d->full = false;
    deque_shrink(d);
    return data;
}

size_t
deque_size(deque_t *d)
{
    if (d->head > d->tail || d->full) {
        return d->capacity - (d->head - d->tail);
    }
    return d->tail - d->head;
}

size_t
deque_item_size(deque_t *d)
{
    return d->item_size;
}

size_t
deque_capacity(deque_t *d)
{
    return d->capacity;
}

int
deque_insert(deque_t *d, void *data, int index)
{
    if (index < 0 || index > deque_size(d)) {
        return -1;
    }
    if (index == deque_size(d)) {
        return deque_push_back(d, data);
    }
    if (index == 0) {
        return deque_push_front(d, data);
    }
    if (deque_expand(d) < 0) {
        return -1;
    }
    size_t idx = deque_shift_right(d, d->head, index);

    if (idx < d->tail) {
        memmove(deque_item_get(d, idx + 1), deque_item_get(d, idx), d->item_size * (d->tail - idx));
        d->tail = deque_shift_right(d, d->tail, 1);
    }
    else {
        size_t left = deque_shift_left(d, d->head, 1);
        memmove(deque_item_get(d, left), deque_item_get(d, d->head), d->item_size * (idx - d->head + 1));
        d->head = left;
    }
    memcpy(deque_item_get(d, idx), data, d->item_size);
    d->full = (d->head == d->tail);
    return 0;
}

void *
deque_remove(deque_t *d, void *data, int index)
{
    if (index < 0 || index >= deque_size(d)) {
        return NULL;
    }
    if (index == deque_size(d) - 1) {
        return deque_pop_back(d, data);
    }
    if (index == 0) {
        return deque_pop_front(d, data);
    }
    if (NULL != data) {
        deque_copy(d, data, index, 1);
    }
    size_t idx = deque_shift_right(d, d->head, index);

    if (idx < d->tail) {
        if (d->tail - idx -1) {
            memmove(deque_item_get(d, idx), deque_item_get(d, idx + 1), d->item_size * (d->tail - idx - 1));
        }
        d->tail = deque_shift_left(d, d->tail, 1);
    }
    else {
        size_t right = deque_shift_right(d, d->head, 1);
        memmove(deque_item_get(d, right), deque_item_get(d, d->head), d->item_size * (idx - d->head));
        d->head = right;
    }
    d->full = false;
    deque_shrink(d);
    return data;
}

void *
deque_get(deque_t *d, int index)
{
    if (index < 0 || index >= deque_size(d)) {
        return NULL;
    }
    size_t idx = deque_shift_right(d, d->head, index);
    return deque_item_get(d, idx);
}

void *
deque_front(deque_t *d)
{
    if (deque_size(d) == 0) {
        return NULL;
    }
    return deque_item_get(d, d->head);
}

void *
deque_back(deque_t *d)
{
    if (deque_size(d) == 0) {
        return NULL;
    }
    size_t last = deque_shift_left(d, d->tail, 1);
    return deque_item_get(d, last);
}

void *
deque_next(deque_t *d, void *current)
{
    char *p = current;
    char *tail = (char *)deque_item_get(d, d->tail);
    char *begin = (char *)(d->array) - d->item_size;
    char *end = (char *)deque_item_get(d, d->capacity);

    if (p + d->item_size == end) {
        p = begin;
    } 
        
    if (p + d->item_size != tail) {
        return p + d->item_size;
    }

    return NULL;
}

void *
deque_previous(deque_t *d, void *current)
{
    char *p = current;
    char *head = (char *)deque_item_get(d, d->head);
    char *begin = (char *)(d->array) - d->item_size;
    char *end = (char *)deque_item_get(d, d->capacity);

    if (p - d->item_size == begin) {
        p = end;
    } 

    if (p - d->item_size != head - d->item_size) {
        return p - d->item_size;
    }

    return NULL;
}

void
deque_swap(deque_t *d, int i, int j)
{
    size_t size = deque_item_size(d);
    size_t x = deque_shift_right(d, d->head, i);
    size_t y = deque_shift_right(d, d->head, j);
    char *a = deque_item_get(d, x);
    char *b = deque_item_get(d, y);

    do {
        char tmp = *a;
        *a++ = *b;
        *b++ = tmp;
    } while (--size > 0);
}

void *
deque_copy(deque_t *d, void *data, int index, int count)
{
    if (index < 0 || count <= 0) {
        return NULL;
    }
    if (index + count > deque_size(d)) {
        return NULL;
    }

    if (NULL == data) {
        return NULL;
    }

    size_t idx = deque_shift_right(d, d->head, index);

    return _deque_copy(d, data, idx, count);
}

void *
deque_detach(deque_t *d)
{
    void *array = NULL;
    if (d->head == 0) {
        array = d->array;
    }
    else {
        array = malloc(d->item_size * deque_size(d));
        if (!_deque_copy(d, array, d->head, deque_size(d))) {
            free(array);
            return NULL;
        }
    }
    d->array = NULL;
    d->head = 0;
    d->tail = 0;
    d->capacity = 0;
    d->full = false;
    return array;
}
