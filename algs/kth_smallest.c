#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void inline
swap(int *a, int *b)
{
    if (a == b) {
        return;
    }

    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void
partition(int *a, int l, int r, int *m1, int *m2)
{
    *m1 = l, *m2 = l;

    for (int i = l + 1; i <= r; ++i) {
        if (a[i] < a[l]) {
            int tmp1 = a[i];
            int tmp2 = a[++(*m2)];
            a[i] = tmp2;
            a[*m2] = a[++(*m1)];
            a[*m1] = tmp1;
        }
        else if (a[i] == a[l]) {
            swap(&a[i], &a[++(*m2)]);
        }
    }

    swap(&a[l], &a[*m1]);
}

static inline int
get_int_random_number(int min, int max)
{
    if (max <= min) {
        return min;
    }
    int number =  min + (rand() % (max - min + 1));
    return number;
}

void
choose_pivot(int *a, int l, int r)
{
    int p = get_int_random_number(l, r);
    swap(&a[l], &a[p]);
}

int
kth_smallest(int *a, int l, int r, int k)
{
    int m1, m2;

    if (l == r) {
        return a[l];
    }

    choose_pivot(a, l, r);

    partition(a, l, r, &m1, &m2);

    if (k > m1 - l && k <= m2 - l + 1) {
        return a[m1];
    }

    if (k <= m1 - l) {
        return kth_smallest(a, l, m1 - 1, k);
    }

    return kth_smallest(a, m2 + 1, r, k - (m2 - l + 1));
}

void
print_array(int *a, int n)
{
    for (int i = 0; i < n; ++i) {
        fprintf(stdout, "%d ", a[i]);
    }
    fprintf(stdout, "\n");
}

int main(int argc, char **argv)
{
    int n, k;
    int *a;

    srand(time(NULL));

    scanf("%d %d", &n, &k);

    a = malloc(n * sizeof(int));

    if (NULL == a) {
        return -1;
    }

    for (int i = 0; i < n; ++i) {
        scanf("%d", &a[i]);
    }

    int kth = kth_smallest(a, 0, n - 1, k);

    fprintf(stdout, "%dth smallest %d\n", k, kth);

    return 0;
}
