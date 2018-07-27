#include <stdio.h>
#include <stdlib.h>

typedef struct segment segment_t;
typedef int (*cmp_t)(int, const segment_t *);

struct segment {
    int low;
    int high;
};

int
cmp_segs_low(const void *v1, const void *v2)
{
    const segment_t *s1 = v1;
    const segment_t *s2 = v2;

    if (s1->low < s2->low) {
        return -1;
    }

    if (s1->low == s2->low) {
        return 0;
    }

    return 1; 
}

int
cmp_point_low(int p, const segment_t *s)
{
    if (p < s->low) {
        return -1;
    }

    if (p == s->low) {
        return 0;
    }

    return 1;
}

int
cmp_rev_segs_high(const void *v1, const void *v2)
{
    const segment_t *s1 = v1;
    const segment_t *s2 = v2;

    if (s1->high > s2->high) {
        return -1;
    }

    if (s1->high == s2->high) {
        return 0;
    }

    return 1; 
}

int
cmp_rev_point_high(int p, const segment_t *s)
{
    if (p > s->high) {
        return -1;
    }

    if (p == s->high) {
        return 0;
    }

    return 1;
}

void 
print_segments(const segment_t *segs, int s)
{
    for (int i = 0; i < s; ++i) {
        fprintf(stdout, "%d %d\n", segs[i].low, segs[i].high);
    }
}

int
binary_search(segment_t *segs, int l, int r, int key, int **index, cmp_t cmp)
{
    int br;

    if (l > r) {
        return -1;
    }

    int m = l + (r - l) / 2;

    int cr = cmp(key, &segs[m]);

    if (cr == 0) {
        while (m + 1 <= r && cmp(key, &segs[m+1]) == 0) ++m;
        *index = malloc(sizeof(int));
        **index = m;
        return m;
    }

    if (cr > 0) {
        br = binary_search(segs, m + 1, r, key, index, cmp);
        if (NULL == *index) {
            *index = malloc(sizeof(int));
            **index = m + 1;
        }
        return br;
    }

    br = binary_search(segs, l, m - 1, key, index, cmp);
    if (NULL == *index) {
        *index = malloc(sizeof(int));
        **index = m;
    }

    return br;
}

int main(int argc, char **argv)
{
    int s, p;

    segment_t *segs1, *segs2;
    int *points;

    scanf("%d %d", &s, &p);

    segs1 = malloc(s * sizeof(segment_t));
    segs2 = malloc(s * sizeof(segment_t));

    points = malloc(p *sizeof(int));

    for (int i = 0; i < s; ++i) {
        scanf("%d %d", &segs1[i].low, &segs1[i].high);
        segs2[i] = segs1[i];
    }

    for (int i = 0; i < p; ++i) {
        scanf("%d", &points[i]);
    }

    qsort(segs1, s, sizeof(segment_t), cmp_segs_low);
    qsort(segs2, s, sizeof(segment_t), cmp_rev_segs_high);

    //print_segments(segs1, s);
    //print_segments(segs2, s);

    /** 
     * n(A Union B) = n(A) + n(B) - n(A Inter B) =>
     * n(A Inter B) = n(A) + n(B) - n(A Union B)
     *
     * A = set of segments having low  end less than or equal p
     * B = set of segments having high end greater than or equal p
     * (A Inter B) = set of segments containing p
     *
     */ 
    for (int i = 0; i < p; ++i) {

        int *l = NULL, *r = NULL;
        int br;

        br = binary_search(segs1, 0, s - 1, points[i], &l, cmp_point_low);
        if (br >= 0) {
            ++(*l);
        }

        br = binary_search(segs2, 0, s - 1, points[i], &r, cmp_rev_point_high);
        if (br >= 0) {
            ++(*r);
        }

        fprintf(stdout, "%d ", *l + *r - s);

        free(l);
        free(r);
    }

    free(segs1);
    free(segs2);
    free(points);

    return 0;
}
