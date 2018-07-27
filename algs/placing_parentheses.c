#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <assert.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

int
digit(const char *expr, int i)
{
    return expr[2*i] - '0';
}

int
operator(const char *expr, int i)
{
    return expr[2*i+1];
}

long int
operation(long int a, long b, char op)
{
    switch (op) {
        case '+':
            return a + b;

        case '-':
            return a - b;

        case '*':
            return a * b;

        default:
            assert(0);
    }
}

void
min_and_max(const char *expr, int i, int j, long int **m, long int **M, long int *min, long int *max)
{
    *min = LONG_MAX;
    *max = LONG_MIN;

    for (int k = i; k < j; ++k) {
        char op = operator(expr, k);

        long int a = operation(M[i][k], M[k+1][j], op); 
        long int b = operation(M[i][k], m[k+1][j], op); 
        long int c = operation(m[i][k], M[k+1][j], op); 
        long int d = operation(m[i][k], m[k+1][j], op);

        *min = MIN(MIN(MIN(MIN(*min, a), b), c), d);
        *max = MAX(MAX(MAX(MAX(*max, a), b), c), d);
    }
}

long int
parenthesis(const char *expr, long int **m, long int **M, long int n)
{
    for (int i = 0; i < n; ++i) {
        m[i][i] = digit(expr, i);
        M[i][i] = digit(expr, i);
    }

    for (int s = 1; s < n; ++s) {
        for (int i = 0; i < n - s; ++i) {
            int j = i + s;
            min_and_max(expr, i, j, m, M, &m[i][j], &M[i][j]);
        }
    }

    return M[0][n - 1];
}

int main(int argc, char **argv)
{
    int n;
    long int **m, **M;
    char expr[30];

    scanf("%s", expr);

    n = (strlen(expr) - 1) / 2 + (strlen(expr) % 2);

    m = malloc(n * sizeof(long int *));
    for (int i = 0; i < n; ++i) {
        m[i] = calloc(n, sizeof(long int));
    }

    M = malloc(n*sizeof(long int *));
    for (int i = 0; i < n; ++i) {
        M[i] = calloc(n, sizeof(long int));
    }

    fprintf(stdout, "%ld\n", parenthesis(expr, m, M, n));

    for (int i = 0; i < n; ++i) {
        free(m[i]);
    }
    free(m);

    for (int i = 0; i < n; ++i) {
        free(M[i]);
    }
    free(M);

    return 0;
}
