#include <stdio.h>
#include <stdlib.h>

int *
sum(int *p1, int *p2, int n)
{
    int *s = calloc(n, sizeof(int));

    for (int i = 0; i < n; ++i) {
        s[i] = p1[i] + p2[i];
    }

    return s;
}

int *
sub(int *p1, int *p2, int n)
{
    int *s = calloc(n, sizeof(int));

    for (int i = 0; i < n; ++i) {
        s[i] = p1[i] - p2[i];
    }

    return s;
}

void
print_array(int *p, int n)
{
    for (int i = 0; i < n; ++i) {
        fprintf(stdout, "%d ", p[i]);
    }
    fprintf(stdout, "\n");
}

void
multiply_wrapper(int *m, int *a, int *b, int n)
{
    if (1 == n) {
        m[0] = a[0] * b[0];
        return;
    }

    /* D0(x) * E0(x) */
    int *m1 = calloc((n-1), sizeof(int));
    multiply_wrapper(m1, a, b, n/2);

    /* D1(x) * E1(x) */
    int *m2 = calloc((n-1), sizeof(int));
    multiply_wrapper(m2, a + n/2, b + n/2, n/2);

    /* D0(x) + D1(x) */
    int *s1 = sum(a, a + n/2, n/2);

    /* E0(x) + E1(x) */
    int *s2 = sum(b, b + n/2, n/2);

    /* (D0(x) + D1(x)) * (E0(x) + E1(x)) */
    int *m3 = calloc((n-1), sizeof(int));
    multiply_wrapper(m3, s1, s2, n/2);

    /* D0(x) * E0(x) + D1(x) * E1(x) */
    int *s3 = sum(m1, m2, n - 1);

    /**
     * karatsuba formula:
     *
     * r =
     * 
     * (D0(x) + D1(x)) * (E0(x) + E1(x)) -
     * (D0(x) * E0(x) + D1(x) * E1(x)) =
     *
     *  D0(x) * E1(x) + D1(x) * E0(x)
     */
    int *r = sub(m3, s3, n - 1);

    for (int i = 0; i < (2 * n - 1); ++i) {
        if (i <= n - 2) {
            m[i] = m1[i];
        }
        if (i >= n) {
            m[i] = m2[i - n];
        }
        if (i >= n/2 && i <= n + n/2 - 2) {
            m[i] += r[i - n/2];
        }
    }

    free(m1);
    free(m2);
    free(m3);
    free(s1);
    free(s2);
    free(s3);
    free(r);
}

int *
multiply(int *a, int *b, int n)
{
    int *m = calloc((2 * n - 1), sizeof(int));
    if (NULL != m) {
        multiply_wrapper(m, a, b, n);
    }
    return m;
}

int main(int argc, char **argv)
{
    int n;
    int *a, *b, *m;

    scanf("%d\n", &n);

    a = calloc(n, sizeof(int));

    if (NULL == a) {
        return -1;
    }

    b = calloc(n, sizeof(int));

    if (NULL == b) {
        return -1;
    }

    for (int i = 0; i < n; ++i) {
        scanf("%d", &a[i]);
    }

    for (int i = 0; i < n; ++i) {
        scanf("%d", &b[i]);
    }

    m = multiply(a, b, n);

    if (NULL == m) {
        return -1;
    }

    for (int i = 0; i < (2 * n - 1); ++i) {
        fprintf(stdout, "%d ", m[i]);
    }
    fprintf(stdout, "\n");
    
    free(m);

    return 0;
}
