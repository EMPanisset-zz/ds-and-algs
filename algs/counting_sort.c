#include <stdlib.h>
#include <stdio.h>

int print_array(int *a, int n)
{
    for (int i = 0; i < n; ++i) {
        fprintf(stdout, "%d ", a[i]);
    }
    fprintf(stdout, "\n");
}

int *counting_sort(int *a, int n, int m)
{
    int *c = calloc(m+1, sizeof(int));
    int *p = calloc(m+1, sizeof(int));
    int *b = malloc(n * sizeof(int));

    /* counter for every occurency of a[i] */
    for (int i = 0; i < n; ++i) {
        ++c[a[i]];
    }

    /* array of positions for each a[i] */
    for (int i = 1; i <= m; ++i) {
        p[i] =  p[i-1] + /**< previous position */
                c[i-1];  /**< number of occurencies of the previous element */
    } 

    for (int i = 0; i < n; ++i) {
        b[(p[a[i]])++] = a[i];
    }

    free(c);
    free(p);

    return b;
}

int main(int argc, char **argv)
{
    int n, *a, *b;

    scanf("%d", &n);

    a = malloc(n * sizeof(int));

    if (NULL == a) {
        return -1;
    }

    for (int i = 0; i < n; ++i) {
        scanf("%d", &a[i]);
    }

    b = counting_sort(a, n, 100);

    print_array(b, n);

    free(a);
    free(b);
    
    return 0;
}
