#include <stdio.h>

static int
gcd(int a, int b)
{
    if (b == 0) {
        return a;
    }

    return gcd(b, a % b);
}

int main(int argc, char **argv)
{
    int a, b;

    scanf("%d %d", &a, &b);

    if (a > 2E10 || a < 1) {
        return -1;
    }

    if (b > 2E10 || b < 1) {
        return -1;
    }

    fprintf(stdout, "%d\n", gcd(a,b));

    return 0;
}
