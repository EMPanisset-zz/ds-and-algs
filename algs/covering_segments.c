#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

typedef struct segment segment_t;

struct segment {
    uint32_t left;
    uint32_t right;
    segment_t *next;
};

int overlap(const segment_t *s1, const segment_t *s2)
{
    if (s1->right < s2->left) {
        return 0;
    }

    if (s2->right < s1->left) {
        return 0;
    }

    return 1;
}

int cmp(const void *v1, const void *v2)
{
    const segment_t *s1 = v1;
    const segment_t *s2 = v2;

    if (s1->right < s2->right) {
        return 1;
    }

    if (s2->right < s1->right) {
        return -1;
    }

    return 0;
}

segment_t *
segment_new(void)
{
    segment_t *s = calloc(1, sizeof(segment_t));
    return s;
}

void
segment_free(segment_t *s)
{
    free(s);
}

segment_t *
segment_min_get(segment_t **previous)
{
    segment_t **prev_min = previous;
    segment_t *min = *previous;

    for (segment_t *s = min; s != NULL; previous = &s->next, s = s->next) {
        if (s->right < min->right) {
            min = s;
            prev_min = previous;
        }  
    }

    if (NULL != min) {
        *prev_min = min->next;
        min->next = NULL;
    }

    return min;
}

int main(int argc, char **argv)
{
    int n;
    segment_t *r = NULL;
    segment_t *segments = NULL;
    segment_t **previous = &segments;
    int cnt = 0;

    scanf("%d", &n);

    for (int i = 0; i < n; ++i) {

        segment_t *segment = segment_new();
        
        scanf("%"PRIu32" %"PRIu32, &segment->left, &segment->right);

        *previous = segment;
        previous = &segment->next;
    }
    *previous = NULL;

    while (segments != NULL) {

        segment_t *min = segment_min_get(&segments);
        segment_t *s = segments;
        segment_t **p = &segments;

        while (s != NULL) {
            if (overlap(min, s)) {
                segment_t *tmp = s->next;
                *p = s->next;
                segment_free(s);
                s = tmp;
            }
            else {
                p = &s->next;
                s = s->next;
            } 
        }

        min->next = r;
        r = min;
        cnt++;
    }

    fprintf(stdout, "%d\n", cnt);

    while (r != NULL) {
        segment_t *tmp = r->next;
        fprintf(stdout, "%"PRIu32" ", r->right);
        free(r);
        r = tmp;
    }
    fprintf(stdout, "\n");

    return 0;
}
