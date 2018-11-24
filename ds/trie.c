#include "includes.h"
#include "trie.h"

#define TRIE_CHILD_OFFSET(idx)  ((idx) & 0x3F)
#define TRIE_CHILD_BITMASK(idx) (UINT64_C(1) << TRIE_CHILD_OFFSET(idx))
#define TRIE_CHILD_CNTMASK(idx) ~(UINT64_C(-1) << TRIE_CHILD_OFFSET(idx))
#define TRIE_DONT_CARE_SYMBOL   '*'
#define TRIE_SLASH_SYMBOL       '/'

#define trie_foreach_child(bitmap, parent) \
        for ((bitmap) = (parent)->bitmap; trie_node_has_child((parent)); \
                trie_node_child_next((parent)))

typedef struct bitmap bitmap_t;
typedef struct trie_node trie_node_t;

struct bitmap {
    uint64_t bits[2]; /**< allows to branch up to 128 children */
};

struct trie_node {
    bitmap_t bitmap; /**< stores set bits for existing children */
    trie_node_t **children;
    slist_t list; /**< allows to store duplicated patterns in the same node */
};

struct trie {
    trie_node_t *root;
    trie_match_cb_t match;
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

static inline void 
trie_node_child_next(trie_node_t *node)
{
    if (node->bitmap.bits[0] > 0) {
        node->bitmap.bits[0] &= node->bitmap.bits[0] - 1;
        return;
    }
    if (node->bitmap.bits[1] > 0) {
        node->bitmap.bits[1] &= node->bitmap.bits[1] - 1;
        return;
    }
}

trie_node_t *
trie_node_new(void)
{
    trie_node_t *node = calloc(1, sizeof(trie_node_t));
    return node;
}

void
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
        while (NULL != slist_pop_front(&node->list)) {
        }
        free(node);
    }
}

trie_t *
trie_new(trie_match_cb_t match)
{
    trie_t *trie = calloc(1, sizeof(trie_t));
    if (NULL != trie) {
        trie->root = trie_node_new();
        if (NULL != trie->root) {
            trie->match = match;
        }
        else {
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

trie_node_t *
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

trie_node_t *
trie_node_child_add(trie_node_t *parent, uint8_t index)
{
    uint8_t child_offset = trie_child_offset(&parent->bitmap, index);
    uint8_t children_count = trie_children_count(&parent->bitmap);

    trie_node_t *child = trie_node_new();

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

int
trie_node_add(trie_node_t *node, const char *key, void *data)
{
    trie_node_t *child = NULL;
    int idx = 0;

    for (idx = 0; key[idx] != '\0'; ++idx) {

        child = trie_node_child(node, key[idx]);

        if (NULL == child) {
            child = trie_node_child_add(node, key[idx]);

            if (NULL == child) {
                return -1;
            }
        }

        node = child;
    }

    if (NULL != child) {
        if (NULL == slist_push_front(&child->list, data)) {
            return -1;
        }
    }

    return 0;
}

static bool 
trie_node_match_all(trie_t *trie, trie_node_t *parent, const char *key, int idx, void *arg, int slash_cnt)
{
    bool matched = false;

    if (NULL == parent) {
        return false;
    }

    if (key[idx] == TRIE_SLASH_SYMBOL) {
        ++slash_cnt;
    } 

    trie_node_t *child = NULL;

    if (key[idx] != '\0' && key[idx] != TRIE_DONT_CARE_SYMBOL) {

        child = trie_node_child(parent, key[idx]);

        if (NULL != child) {
            if (NULL != slist_head(&child->list) && key[idx+1] == '\0') {
                snode_t *node;
                slist_foreach(&child->list, node) {
                    trie->match(snode_data(node), arg);
                }
                matched = true;
            }
            if (trie_node_match_all(trie, child, key, idx + 1, arg, slash_cnt)) {
                matched = true;
            }
        }
    }

    child = trie_node_child(parent, TRIE_DONT_CARE_SYMBOL);

    if (NULL != child) {

        if (NULL != slist_head(&child->list)) {
            snode_t *node;
            slist_foreach(&child->list, node) {
                trie->match(snode_data(node), arg);
            }
            matched = true;
        }
        /* Iterates over all DON'T CARE symbol's children */
        bitmap_t bitmap; /**< used to save original bitmap */
        trie_foreach_child(bitmap, child) {
            int j = idx;
            while (key[j] != '\0' && (key[j] != TRIE_SLASH_SYMBOL || slash_cnt > 1)) {
                if (trie_node_match_all(trie, child, key, j++, arg, slash_cnt)) {
                    /* branch already successful matched */
                    matched = true;
                    break;
                }
            }
        }
        /* put back original bitmap */
        child->bitmap = bitmap;
    }

    if (key[idx] == TRIE_SLASH_SYMBOL) {
        --slash_cnt;
    } 

    return matched;
}

int
trie_add(trie_t *trie, const char *key, void *data)
{
    return trie_node_add(trie->root, key, data); 
}

bool
trie_match_all(trie_t *trie, const char *key, void *arg)
{
    return trie_node_match_all(trie, trie->root, key, 0, arg, 0);
}
