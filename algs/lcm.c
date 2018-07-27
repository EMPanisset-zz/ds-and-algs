#include <stdio.h>

static int
gcd(int a, int b)
{
    if (b == 0) {
        return a;
    }

    return gcd(b, a % b);
}

static unsigned long 
lcm(int a, int b)
{
    return (unsigned long)a * b / gcd(a, b);
}

int main(int argc, char **argv)
{
    int a, b;

    scanf("%d %d", &a, &b);

    if (a > 2E9 || a < 1) {
        return -1;
    }

    if (b > 2E9 || b < 1) {
        return -1;
    }

    fprintf(stdout, "%ld\n", lcm(a,b));

    return 0;
}
