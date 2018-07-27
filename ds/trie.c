#include "includes.h"
#include "trie.h"

#define TRIE_CHILD_OFFSET(idx)  ((idx) & 0x3F)
#define TRIE_CHILD_BITMASK(idx) (UINT64_C(1) << TRIE_CHILD_OFFSET(idx))
#define TRIE_CHILD_CNTMASK(idx) ~(UINT64_C(-1) << TRIE_CHILD_OFFSET(idx))

#define trie_foreach_child(parent, child, bitmap, pos) \
        for ((bitmap) = (parent)->bitmap, \
             (pos) = bitmap_ffs(&(bitmap)); \
             (pos) > 0 && ((child) = trie_node_child(parent, (pos) - 1)); \
             bitmap_next(&(bitmap)), (pos) = bitmap_ffs(&(bitmap)))

typedef struct bitmap bitmap_t;
typedef struct trie_node trie_node_t;

struct bitmap {
    uint64_t bits[2]; /**< allows to branch up to 128 children */
};

struct trie_node {
    bitmap_t bitmap; /**< stores set bits for existing children */
    trie_node_t **children;
    int id;
    void *data;
};

struct trie {
    trie_node_t *root;
    int last_id;
};

/**
 * Returns number of set bits
 *
 * Implementation uses magic numbers to get things done quickly
 *
 * Ref: https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
 */
static inline unsigned int
popcount64(uint64_t v)
{
    unsigned int c;

    v = v - ((v >> 1) & UINT64_C(0x5555555555555555));

    v = (v & UINT64_C(0x3333333333333333)) +
        ((v >> 2) & UINT64_C(0x3333333333333333));

    v = ((v + (v >> 4)) & UINT64_C(0x0f0f0f0f0f0f0f0f)) *
        UINT64_C(0x0101010101010101);

    c = (unsigned int)(v >> 56);

    return c;
}

static inline void 
bitmap_next(bitmap_t *bitmap)
{
    if (bitmap->bits[0] > 0) {
        bitmap->bits[0] &= bitmap->bits[0] - 1;
        return;
    }
    if (bitmap->bits[1] > 0) {
        bitmap->bits[1] &= bitmap->bits[1] - 1;
        return;
    }
}

static inline int
bitmap_ffs(bitmap_t *bitmap)
{
    if (bitmap->bits[0] > 0) {
        return ffsll(bitmap->bits[0]);
    }
    if (bitmap->bits[1] > 0) {
        return 64 + ffsll(bitmap->bits[1]);
    }
    return 0;
}

static inline void
trie_child_bit_set(bitmap_t *bitmap, uint8_t index)
{
    bitmap->bits[index >> 6] |= TRIE_CHILD_BITMASK(index);
}

static inline void
trie_child_bit_clear(bitmap_t *bitmap, uint8_t index)
{
    bitmap->bits[index >> 6] &= ~TRIE_CHILD_BITMASK(index);
}

static inline bool
trie_child_is_bit_set(bitmap_t *bitmap, uint8_t index)
{
    return !!(bitmap->bits[index >> 6] & TRIE_CHILD_BITMASK(index));
}

static inline uint8_t
trie_children_count(bitmap_t *bitmap)
{
    return popcount64(bitmap->bits[0]) + popcount64(bitmap->bits[1]);
}

static inline uint8_t 
trie_child_offset(bitmap_t *bitmap, uint8_t index)
{
    uint8_t offset = 0;
    if (index >= 64) {
        offset += popcount64(bitmap->bits[0]);
        offset += popcount64(bitmap->bits[1] & TRIE_CHILD_CNTMASK(index));
    }
    else {
        offset += popcount64(bitmap->bits[0] & TRIE_CHILD_CNTMASK(index));
    }
    return offset;
}

static inline uint8_t
trie_node_has_child(trie_node_t *node)
{
    if (node->bitmap.bits[0] > 0) {
        return true;
    }

    if (node->bitmap.bits[1] > 0) {
        return true;
    }

    return false;
}

static trie_node_t *
trie_node_new(trie_t *trie)
{
    trie_node_t *node = calloc(1, sizeof(trie_node_t));
    if (NULL != node) {
        node->id = trie->last_id++;
    }
    return node;
}

static void
trie_node_free(trie_node_t *node)
{
    if (NULL != node) {
        uint8_t children_count = trie_children_count(&node->bitmap);
        int i = 0;
        for (i = 0; i < children_count; ++i) {
            trie_node_free(node->children[i]);
        }
        memset(&node->bitmap, 0, sizeof(bitmap_t));
        free(node->children);
        node->children = NULL;
        free(node);
    }
}

trie_t *
trie_new(void)
{
    trie_t *trie = calloc(1, sizeof(trie_t));
    if (NULL != trie) {
        trie->root = trie_node_new(trie);
        if (NULL == trie->root) {
            free(trie);
            trie = NULL;
        }
    }
    return trie;
}

void
trie_free(trie_t *trie)
{
    if (NULL != trie) {
        trie_node_free(trie->root);
        free(trie);
    }
}

static bool
trie_node_end_of_pattern(trie_node_t *node)
{
    return (node->data != NULL);
}

static trie_node_t *
trie_node_child(trie_node_t *parent, uint8_t index)
{
    if (NULL != parent) {
        if (trie_child_is_bit_set(&parent->bitmap, index)) {
            uint8_t offset;
            offset = trie_child_offset(&parent->bitmap, index);
            return parent->children[offset];
        }
    }
    return NULL;
}

static trie_node_t *
trie_node_child_add(trie_t *trie, trie_node_t *parent, uint8_t index)
{
    uint8_t child_offset = trie_child_offset(&parent->bitmap, index);
    uint8_t children_count = trie_children_count(&parent->bitmap);

    trie_node_t *child = trie_node_new(trie);

    if (NULL == child) {
        return NULL;
    }

    trie_node_t **children = realloc(parent->children, (children_count + 1) * sizeof(trie_node_t*));

    if (NULL == children) {
        goto error;
    }

    if (child_offset < children_count) {
        memmove(children + child_offset + 1,
                children + child_offset,
                (children_count - child_offset) * sizeof(trie_node_t*));
    }

    children[child_offset] = child;

    trie_child_bit_set(&parent->bitmap, index);

    parent->children = children;

    return child;

error:
    trie_node_free(child);

    return NULL;
}

static int
trie_node_add(trie_t *trie, trie_node_t *node, const char *key, void *data)
{
    trie_node_t *child = NULL;
    int idx = 0;

    for (idx = 0; key[idx] != '\0'; ++idx) {

        child = trie_node_child(node, key[idx]);

        if (NULL == child) {
            child = trie_node_child_add(trie, node, key[idx]);

            if (NULL == child) {
                return -1;
            }
        }

        node = child;
    }

    if (NULL != child) {
        child->data = data;
    }

    return 0;
}

int
trie_add(trie_t *trie, const char *key, void *data)
{
    return trie_node_add(trie, trie->root, key, data); 
}

static void
trie_node_print(trie_node_t *node)
{
    int pos;
    trie_node_t *child = NULL;
    bitmap_t bitmap;

    trie_foreach_child(node, child, bitmap, pos) {
        fprintf(stdout, "%d->%d:%c\n", node->id, child->id, (char)(pos - 1));
        trie_node_print(child);
    }
}

void
trie_print(trie_t *trie)
{
    trie_node_print(trie->root);
}

void
trie_match_all(trie_t *trie, const char *text, trie_match_fn_t match, void *arg)
{
    int i;
    for (i = 0; text[i] != '\0'; ++i) {
        trie_node_t *root = trie->root;
        int j;
        for (j = i; text[j] != '\0'; ++j) {
            trie_node_t *child = trie_node_child(root, text[j]);
            if (NULL == child) {
                break;
            }
            if (trie_node_end_of_pattern(child)) {
                match(i, j, arg);
                break;
            }
            root = child;
        }
    }
}

static void
match_fn(int i, int j, void *arg)
{
    fprintf(stdout, "%d ", i);
}

int main(int argc, char **argv)
{
    int i, n;
    char *p;
    char *text;

    trie_t *trie = trie_new();

    scanf("%ms", &text);

    scanf("%d", &n);

    for (i = 0; i < n; ++i) {
        scanf("%ms", &p);
        trie_add(trie, p, p);
        free(p);
    }

    trie_match_all(trie, text, match_fn, NULL);
    fprintf(stdout, "\n");
    trie_free(trie);

    free(text);

    return 0;
}
