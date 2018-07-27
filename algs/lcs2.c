#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(a,b) ((a) > (b) ? (a) : (b))

int
lcs(int *a, int n, int *b, int m)
{
    int ** value = malloc((n + 1) * sizeof(int *));

    for (int i = 0; i < n + 1; ++i) {
        value[i] = calloc(m + 1, sizeof(int));
    }

    for (int j = 1; j < m + 1; ++j) {
        for (int i = 1; i < n + 1; ++i) {
            int insertion = value[i][j-1];
            int deletion = value[i-1][j];
            int match = value[i-1][j-1] + 1;
            int mismatch = value[i-1][j-1];
            if (a[i-1] == b[j-1]) {
                value[i][j] = match;
                value[i][j] = MAX(insertion, MAX(deletion, match));
            }
            else {
                value[i][j] = MAX(insertion, MAX(deletion, mismatch));
            }
        }
    }

    for (int i = 0; i < n + 1; ++i) {
        for (int j = 0; j < m + 1; ++j) {
            fprintf(stdout, "%d ", value[i][j]);
        }
        fprintf(stdout, "\n");
    }

    int d = value[n][m];

    for (int i = 0; i < n + 1; ++i) {
        free(value[i]);
    }
    free(value);

    return d;
}

int main(int argc, char **argv)
{
    int n, m;

    int *a, *b;

    scanf("%d", &n);

    a = malloc(n * sizeof(int));

    getchar();

    for (int i = 0; i < n; ++i) {
        scanf("%d", &a[i]);
    }

    scanf("%d", &m);

    b = malloc(m * sizeof(int));

    getchar();

    for (int j = 0; j < m; ++j) {
        scanf("%d", &b[j]);
    }

    fprintf(stdout, "%d\n", lcs(a, n, b, m));

    free(a);
    free(b);

    return 0;
}
