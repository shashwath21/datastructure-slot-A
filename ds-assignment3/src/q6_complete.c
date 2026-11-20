#include <stdio.h>
#include <stdlib.h>
typedef struct Trie {
    struct Trie *child[26];
    int words;                              /* number of words passing through here */
    int end;                                /* 1 if a word ends here */
} Trie;
Trie *node(void) { return calloc(1, sizeof(Trie)); }

void insert(Trie *t, const char *w)
{
    /* TODO 1 - done */
    for (; *w; w++) {
        int i = *w - 'a';
        if (!t->child[i]) t->child[i] = node();
        t = t->child[i];
        t->words++;
    }
    t->end = 1;
}
int count_prefix(const Trie *t, const char *p)
{
    /* TODO 2 - done */
    for (; *p; p++) {
        t = t->child[*p - 'a'];
        if (!t) return 0;
    }
    return t->words;
}
void destroy(Trie *t)
{
    if (!t) return;
    for (int i = 0; i < 26; i++) destroy(t->child[i]);
    free(t);
}
static int count_nodes(const Trie *t) { if (!t) return 0; int c = 1; for (int i = 0; i < 26; i++) c += count_nodes(t->child[i]); return c; }

int main(void)
{
    const char *contacts[] = {"anand", "anitha", "anu", "arun", "bala", "anusuya"};
    Trie *root = node();
    for (int i = 0; i < 6; i++) insert(root, contacts[i]);
    const char *typed[] = {"a", "an", "anu", "b", "z"};
    for (int i = 0; i < 5; i++)
        printf("%s:%d ", typed[i], count_prefix(root, typed[i]));
    printf("\n");
    int nodes = count_nodes(root);              /* includes root */
    printf("nodes = %d (incl. root), memory = %d x %zu bytes = %zu bytes\n", nodes, nodes, sizeof(Trie), nodes * sizeof(Trie));
    destroy(root);
    return 0;
}
