#include <stdio.h>
#include <stdlib.h>

typedef struct item item_t;

struct item {
    int w;
    int v;
};

int main(int argc, char **argv)
{
    int W, n;
    int **value = NULL;
    item_t *items = NULL;

    scanf("%d %d", &W, &n);

    items = calloc(n + 1, sizeof(item_t));

    for (int i = 1; i < n + 1; ++i) {
        scanf("%d", &items[i].w);
        items[i].v = items[i].w;
    }

    value = malloc((n + 1) * sizeof(int *));

    for (int i = 0; i < n + 1; ++i) {
        value[i] = calloc(W + 1, sizeof(int));
    }

    for (int i = 1; i < n + 1; ++i) {
        for (int w = 1; w < W + 1; ++w) {
            value[i][w] = value[i-1][w];
            if (items[i].w <= w) {
                int val = value[i - 1][w - items[i].w] + items[i].v;
                if (value[i][w] < val) {
                    value[i][w] = val;
                }
            }
        }
    }

    /*
    for (int i = 0; i < n + 1; ++i) {
        for (int j = 0; j < W + 1; ++j) {
            fprintf(stderr, "%d ", value[i][j]);
        }
        fprintf(stderr, "\n");
    }*/

    fprintf(stdout, "%d\n", value[n][W]);

    return 0;
}
