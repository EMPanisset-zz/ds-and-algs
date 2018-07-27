#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

typedef struct number number_t;


struct number {
    int v;
    number_t *next;
    int count;
    uint8_t digits[6];
};

int
number_digits_count(number_t *n)
{
    int cnt = 1;
    int v = n->v;

    while (v / 10 > 0) {
        ++cnt;
        v /= 10;
    }

    //fprintf(stdout, "digits %d -> %d\n", n->v, cnt);

    return cnt;
}

void
number_shift_right(number_t *number, int n)
{
    uint8_t prev = number->digits[0];
    for (int j = 1; j <= n; ++j) {
        uint8_t current = number->digits[j];
        number->digits[j] = prev;
        prev = current;
    }
}

number_t *
number_new(int v) {
    number_t *number = calloc(1, sizeof(number_t));
    number->v = v;
    while (v/ 10 > 0) {
        number_shift_right(number, number->count++);
        number->digits[0] = v % 10;
        v /= 10;
    }
    number_shift_right(number, number->count++);
    number->digits[0] = v % 10;
    return number;
}

void
number_free(number_t *n) {
    free(n);
}

int
is_greater_or_equal(number_t *n, number_t *m)
{
    if (n->count == m->count) {
        return n->v >= m->v;
    }

    number_t *p1 = n;
    uint8_t *d1 = &n->digits[0];
    uint8_t *d2 = &m->digits[0];
    int c1 = n->count;
    int c2 = m->count;

    if (n->count < m->count) {
        p1 = m;
        d1 = &m->digits[0];
        d2 = &n->digits[0];
        c1 = m->count;
        c2 = n->count;
    }

    for (int i = 0; i < c1 + c2; ++i) {

        if (*d1 > *d2) {
           return p1 == n;
        }

        if (*d1 < *d2) {
            return p1 != n;
        }

        if (i+1 == c1) {
            if (p1 == n) {
                d1 = &m->digits[0];
            }
            else {
                d1 = &n->digits[0];
            }
        }
        else {
            ++d1;
        }

        if (i+1 == c2) {
            if (p1 == n) {
                d2 = &n->digits[0];
            }
            else {
                d2 = &m->digits[0];
            }
        }
        else {
            ++d2;
        }
    }

    return 1;
}

number_t *
largest_number(number_t *numbers, int n)
{
    number_t *answer = NULL;
    number_t **last = &answer;

    while (NULL != numbers) {
        number_t *m = NULL;
        number_t *max = NULL;
        number_t **prev_max = &numbers;
        number_t **p = &numbers;
        for (m = numbers; m != NULL; p = &m->next, m = m->next) {
            if (NULL == max || is_greater_or_equal(m, max)) {
                max = m;
                prev_max = p;
            }
        }
        *prev_max = max->next;
        max->next = NULL;
        *last = max;
        last = &max->next;
    }

    return answer;
}
                
int main(int argc, char **argv)
{
    int n;
    number_t *answer;
    number_t *number;
    number_t *numbers;
    number_t **p = &numbers;

    scanf("%d", &n);

    if (n < 1 || n > 100) {
        return -1;
    }

    for (int i = 0; i < n; ++i) {
        int v;
        scanf("%d", &v);

        if (v < 1 || v > 100000) {
            return -1;
        }

        number = number_new(v);
        *p = number;
        p = &number->next;
    }

    answer = largest_number(numbers, n);

    for(number = answer; number != NULL; number = number->next) {
        fprintf(stdout, "%d", number->v);
    }
    fprintf(stdout, "\n"); 
    
    return 0;
}
