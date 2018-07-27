#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(a,b) (a < b ? a : b)

int main(int argc, char **argv)
{
    int len1, len2;
    char string1[101], string2[101];
    int **value;

    memset(string1, 0, sizeof(string1));
    memset(string2, 0, sizeof(string2)); 

    scanf("%s", string1);
    scanf("%s", string2);

    len1 = strlen(string1);
    len2 = strlen(string2);

    value = malloc((len1 + 1) * sizeof(int *));

    for (int i = 0; i < len1 + 1; ++i) {
        value[i] = calloc(len2 + 1, sizeof(int));
    }

    for (int i = 0; i < len1 + 1; ++i) {
        value[i][0] = i;
    }
    for (int j = 0; j < len2 + 1; ++j) {
        value[0][j] = j;
    }

    for (int j = 1; j < len2 + 1; ++j) {
        for (int i = 1; i < len1 + 1; ++i) {
            int insertion = value[i][j-1] + 1;
            int deletion = value[i-1][j] + 1;
            int match = value[i-1][j-1];
            int mismatch = value[i-1][j-1] + 1;
            if (string1[i-1] == string2[j-1]) {
                value[i][j] = MIN(insertion, MIN(deletion, match));
            }
            else {
                value[i][j] = MIN(insertion, MIN(deletion, mismatch));
            }
        }
    }

    /*
    for (int i = 0; i < len1 + 1; ++i) {
        for (int j = 0; j < len2 + 1; ++j) {
            fprintf(stdout, "%d ", value[i][j]);
        }
        fprintf(stdout, "\n");
    }
    */

    fprintf(stdout, "%d\n", value[len1][len2]);

    return 0;
}
