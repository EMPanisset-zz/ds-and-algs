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

void
quick_sort(int *a, int l, int r)
{
    int m1, m2;

    if (l > r) {
        return;
    }

    choose_pivot(a, l, r);

    partition(a, l, r, &m1, &m2);

    quick_sort(a, l, m1 - 1);
    quick_sort(a, m2 + 1, r);
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
    int n;
    int *a;

    srand(time(NULL));

    scanf("%d", &n);

    a = malloc(n * sizeof(int));

    if (NULL == a) {
        return -1;
    }

    for (int i = 0; i < n; ++i) {
        scanf("%d", &a[i]);
    }

    quick_sort(a, 0, n - 1);

    print_array(a, n);

    return 0;
}
