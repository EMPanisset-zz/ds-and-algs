#include "includes.h"
#include "array.h"

void
array_print(array_t *array)
{
    int *current;
    array_foreach(array, current) {
        printf("%d ", *current);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    int a[] = { 1, 2, 3, 4, 5 };
    array_t *array = array_new(0, sizeof(int));

    int i;
    for (i = 0; i < countof(a); ++i) {
        array_push_back(array, &a[i]);
    }

    printf("%zu\n", array_size(array));
   
    array_print(array); 

    while (array_size(array)) {
        int e;
        array_pop_back(array, &e); 
        printf("%d ", e);
    }
    printf("\n");
    
    printf("%zu\n", array_size(array));

    for (i = 0; i < countof(a); ++i) {
        array_push_back(array, &a[i]);
    }

    printf("%zu\n", array_capacity(array));

    array_swap(array, 0, countof(a) - 1);

    int *e1 = array_get(array, 0);
    int *e2 = array_get(array, countof(a) - 1);

    printf("%d %d\n", *e1, *e2);

    int e3 = 6;
    array_insert(array, &e3, array_size(array) / 2);

    array_print(array);

    printf("%zu\n", array_capacity(array));

    array_remove(array, &e3, array_size(array) / 2);

    array_print(array);

    array_copy(array, &e3, array_size(array) / 2);

    printf("%d\n", e3);

    array_free(array);
    return 0;
}
