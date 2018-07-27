#include <stdio.h>
#include <stdlib.h>

int *
merge(const int *a1, int a1_len, const int *a2, int a2_len)
{
    int * r = malloc((a1_len + a2_len) * sizeof(int));

    int i = 0, j = 0, k = 0;

    while (i < a1_len && j < a2_len) {
        if (a1[i] <= a2[j]) {
            r[k++] = a1[i++];
        }
        else {
            r[k++] = a2[j++];
        }
    }

    while (i < a1_len) {
        r[k++] = a1[i++];
    }
    
    while (j < a2_len) {
        r[k++] = a2[j++];
    }

    return r;
}

int *
mergesort(const int *a, int n)
{
    int *a1, *a2, *r;

    if (1 == n) {
        r = malloc(sizeof(int));
        r[0] = a[0];
        return r;
    }

    int a1_len = n/2;
    int a2_len = n - n/2;

    a1 = mergesort(a, a1_len);
    a2 = mergesort(a + n/2, a2_len);

    r = merge(a1, a1_len, a2, a2_len);

    free(a1);
    free(a2);

    return r;
}

int main(int argc, char **argv)
{
    int n;
    int *a;

    scanf("%d", &n);

    a = malloc(n * sizeof(int));

    if (NULL == a) {
        return -1;
    }

    for (int i = 0; i < n; ++i) {
        scanf("%d", &a[i]);
    }

    int *r = mergesort(a, n);

    for (int i = 0; i < n; ++i) {
        fprintf(stdout, "%d ", r[i]);
    }
    fprintf(stdout, "\n");

    free(r);

    return 0;
}
