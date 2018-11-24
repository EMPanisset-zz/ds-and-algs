#include "includes.h"
#include "deque.h"

void
deque_print(deque_t *deque)
{
    int *current;
    deque_foreach(deque, current) {
        printf("%d ", *current);
    }
    printf("\n");
}

typedef struct pair pair_t;

struct pair {
    int first;
    int second;
};

pair_t *
pair_new(int first, int second)
{
    pair_t *pair = calloc(1, sizeof(*pair));
    pair->first = first;
    pair->second = second;
    return pair;
}

void
pair_free(pair_t *pair)
{
    free(pair);
}

int*
max_sliding_window(int* nums, int n, int k, int* returnSize) {
 
    int *r = NULL;
    deque_t *window = deque_new(0, sizeof(pair_t *));
    
    *returnSize = n - k + 1;
    
    r = malloc((*returnSize) * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        pair_t *pair = NULL;
        pair_t **ppair = NULL;
        
        ppair = deque_back(window);
        while (ppair && (*ppair)->first <= nums[i]) {
            deque_pop_back(window, &pair);
            pair_free(pair);
            ppair = deque_back(window);
        }
        
        pair = pair_new(nums[i], i);
        deque_push_back(window, &pair);

        ppair = deque_front(window);
        while(ppair && (*ppair)->second <= i - k) {
            deque_pop_front(window, &pair);
            pair_free(pair);
            ppair = deque_front(window);
        }

        if (i >= k - 1) {
            ppair = deque_front(window);
            r[i - k + 1] = (*ppair)->first;
        }       
    }
    
    deque_free(window);
    
    return r;
}

int main(int argc, char **argv)
{
    int a[] = { 1, 2, 3, 4, 5 };
    deque_t *deque = deque_new(0, sizeof(int));

    int i;
    for (i = 0; i < countof(a); ++i) {
        deque_push_back(deque, &a[i]);
    }

    printf("%zu\n", deque_size(deque));
   
    deque_print(deque); 

    while (deque_size(deque)) {
        int e;
        deque_pop_back(deque, &e); 
        printf("%d ", e);
    }
    printf("\n");
    
    printf("%zu\n", deque_size(deque));

    for (i = 0; i < countof(a); ++i) {
        deque_push_back(deque, &a[i]);
    }

    printf("%zu\n", deque_capacity(deque));

    deque_swap(deque, 0, countof(a) - 1);

    int *e1 = deque_get(deque, 0);
    int *e2 = deque_get(deque, countof(a) - 1);

    printf("%d %d\n", *e1, *e2);

    int e3 = 6;
    deque_insert(deque, &e3, deque_size(deque) / 2);

    deque_print(deque);

    printf("%zu\n", deque_capacity(deque));

    deque_remove(deque, &e3, deque_size(deque) / 2);

    deque_print(deque);

    deque_copy(deque, &e3, deque_size(deque) / 2, 1);

    printf("%d\n", e3);

    for (i = 7; i < 12; ++i) {
        deque_push_front(deque, &i);
    }

    while (deque_size(deque)) {
        int e;
        deque_pop_front(deque, &e); 
        printf("%d ", e);
    }
    printf("\n");

    deque_print(deque);

    deque_free(deque);

    int b[] = { 5, 4, 1, 3, 2, 0, 1, 4, 6 };
    int m;

    int *max = max_sliding_window(b, countof(b), 3, &m);

    for (i = 0; i < m; ++i) {
        printf("%d ", max[i]);
    }
    printf("\n");

    free(max);
    return 0;
}
