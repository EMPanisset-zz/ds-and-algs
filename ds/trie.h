#ifndef _TRIE__H__
#define _TRIE__H__

#include "includes.h"

typedef struct trie trie_t;

typedef void (*trie_match_fn_t)(int, int, void *);

trie_t *
trie_new(void);

void
trie_free(trie_t *trie);

int
trie_add(trie_t *trie, const char *key, void *data);

void
trie_match_all(trie_t *trie, const char *text, trie_match_fn_t match, void *arg);

void
trie_print(trie_t *trie);

#endif /* _TRIE__H__ */
