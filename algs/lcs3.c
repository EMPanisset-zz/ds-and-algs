#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(a,b) ((a) > (b) ? (a) : (b))

int
lcs(int *a, int n, int *b, int m, int *c, int l)
{
    int *** value = malloc((n + 1) * sizeof(int *));

    for (int i = 0; i < n + 1; ++i) {
        value[i] = malloc((m + 1) * sizeof(int *));
        for (int j = 0; j < m + 1; ++j) {
            value[i][j] = calloc(l + 1, sizeof(int));
        }
    }

    for (int k = 1; k < l + 1; ++k) {
        for (int j = 1; j < m + 1; ++j) {
            for (int i = 1; i < n + 1; ++i) {
                int insertion1 = value[i][j-1][k];
                int insertion2 = value[i][j][k-1];
                int deletion = value[i-1][j][k];
                int match = value[i-1][j-1][k-1] + 1;
                int mismatch = value[i-1][j-1][k-1];
                if ((a[i-1] == b[j-1]) && (b[j-1] == c[k-1])) {
                    value[i][j][k] = MAX(insertion2, MAX(insertion1, MAX(deletion, match)));
                }
                else {
                    value[i][j][k] = MAX(insertion2, MAX(insertion1, MAX(deletion, mismatch)));
                }
            }
        }
    }

    /*
    for (int i = 0; i < n + 1; ++i) {
        for (int j = 0; j < m + 1; ++j) {
            for (int k = 0; k < l + 1; ++k) {
                fprintf(stdout, "%d ", value[i][j][k]);
            }
            fprintf(stdout, "\n");
        }
    }
    */

    int d = value[n][m][l];

    for (int i = 0; i < n + 1; ++i) {
        for (int j = 0; j < m + 1; ++j) {
            free(value[i][j]);
        }
        free(value[i]);
    }
    free(value);

    return d;
}

int main(int argc, char **argv)
{
    int n, m, l;

    int *a, *b, *c;

    scanf("%d", &n);

    a = malloc(n * sizeof(int));

    for (int i = 0; i < n; ++i) {
        scanf("%d", &a[i]);
    }

    scanf("%d", &m);

    b = malloc(m * sizeof(int));

    for (int j = 0; j < m; ++j) {
        scanf("%d", &b[j]);
    }

    scanf("%d", &l);

    c = malloc(l * sizeof(int));

    for (int k = 0; k < l; ++k) {
        scanf("%d", &c[k]);
    }

    fprintf(stdout, "%d\n", lcs(a, n, b, m, c, l));

    free(a);
    free(b);
    free(c);

    return 0;
}
