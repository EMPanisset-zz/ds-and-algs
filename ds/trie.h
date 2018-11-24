#ifndef _TRIE__H__
#define _TRIE__H__

#include "includes.h"

typedef struct trie trie_t;

typedef void (*trie_match_cb_t)(void *, void *);

trie_t *
trie_new(trie_match_cb_t match);

void
trie_free(trie_t *trie);

int
trie_add(trie_t *trie, const char *key, void *data);

bool
trie_match_all(trie_t *trie, const char *key, void *arg);

#endif /* _TRIE__H__ */
