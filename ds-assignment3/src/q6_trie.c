/* Q6 - Trie: insert, search, prefix count, autocomplete, delete, 5,000-name load and timings */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct Trie { struct Trie *child[26]; int words; int end; } Trie;
static Trie *node(void) { return calloc(1, sizeof(Trie)); }
static long nodes_alive = 0;

static int search(const Trie *t, const char *w) {
    for (; *w; w++) { t = t->child[*w - 'a']; if (!t) return 0; }
    return t->end;
}
static void insert(Trie *t, const char *w) {
    if (search(t, w)) return;                          /* duplicate word: do not double count */
    t->words++;                                        /* root counts all words (empty prefix) */
    for (; *w; w++) {
        int i = *w - 'a';
        if (!t->child[i]) { t->child[i] = node(); nodes_alive++; }
        t = t->child[i]; t->words++;
    }
    t->end = 1;
}
static int count_prefix(const Trie *t, const char *p) {
    for (; *p; p++) { t = t->child[*p - 'a']; if (!t) return 0; }
    return t->words;
}
static void collect(const Trie *t, char *buf, int d, int *printed, int limit, int quiet) {
    if (t->end) { if (!quiet && *printed < limit) printf("%.*s ", d, buf); (*printed)++; }
    for (int i = 0; i < 26; i++) if (t->child[i]) { buf[d] = 'a' + i; collect(t->child[i], buf, d + 1, printed, limit, quiet); }
}
/* alphabetical because children are visited a..z */
static int autocomplete(const Trie *t, const char *prefix, int limit, int quiet) {
    char buf[128]; strcpy(buf, prefix);
    for (const char *p = prefix; *p; p++) { t = t->child[*p - 'a']; if (!t) return 0; }
    int printed = 0; collect(t, buf, (int)strlen(prefix), &printed, limit, quiet);
    return printed;
}
/* delete: walk down decrementing 'words'; the first node whose count drops to 0 roots a chain that
   served only this word, so that whole chain is freed (pruning). Otherwise just clear 'end'. */
static void free_chain(Trie *t) { if (!t) return; for (int i = 0; i < 26; i++) free_chain(t->child[i]); free(t); nodes_alive--; }
static int delete_word(Trie *root, const char *w) {
    if (!search(root, w)) return 0;
    Trie *t = root; root->words--;
    for (; *w; w++) {
        int i = *w - 'a'; Trie *c = t->child[i];
        if (--c->words == 0) { free_chain(c); t->child[i] = NULL; return 1; }
        t = c;
    }
    t->end = 0;                                         /* word was a prefix of longer words: keep the nodes */
    return 1;
}
static void destroy(Trie *t) { if (!t) return; for (int i = 0; i < 26; i++) destroy(t->child[i]); free(t); }

/* ---- sample name generator (deterministic) ---- */
static const char *SYL[] = {"an","ar","ba","bha","chi","da","dha","ga","ha","ja","ka","ki","la","li","ma","mi","na","ni","pa","pri","ra","ri","sa","shi","ta","thi","va","vi","ya","ku","mu","su","re","ve"};
static void make_name(char *out) {
    int syl = 2 + rand() % 3; out[0] = 0;
    for (int i = 0; i < syl; i++) strcat(out, SYL[rand() % 34]);
}
static double now_s(void) { struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); return ts.tv_sec + ts.tv_nsec * 1e-9; }
static int cmp_str(const void *a, const void *b) { return strcmp(*(char *const *)a, *(char *const *)b); }

int main(void) {
    srand(2024);
    puts("=== Small demo: the six contacts ===");
    const char *contacts[] = {"anand", "anitha", "anu", "arun", "bala", "anusuya"};
    Trie *r = node(); nodes_alive = 1;
    for (int i = 0; i < 6; i++) insert(r, contacts[i]);
    printf("nodes = %ld, memory = %ld x %zu = %zu bytes\n", nodes_alive, nodes_alive, sizeof(Trie), nodes_alive * sizeof(Trie));
    const char *pf[] = {"a", "an", "anu", "b", "z", ""};
    for (int i = 0; i < 6; i++) printf("count_prefix(\"%s\") = %d\n", pf[i], count_prefix(r, pf[i]));
    printf("autocomplete \"an\"  : "); autocomplete(r, "an", 100, 0);  puts("");
    printf("autocomplete \"anu\" : "); autocomplete(r, "anu", 100, 0); puts("");
    printf("autocomplete \"anuq\": (no matches) -> %d\n", autocomplete(r, "anuq", 100, 0));
    printf("autocomplete \"anitha\" (prefix is a whole word): "); autocomplete(r, "anitha", 100, 0); puts("");
    printf("autocomplete \"\" (empty prefix lists everything): "); autocomplete(r, "", 100, 0); puts("");
    int d1 = delete_word(r, "anu");               /* separate statements: printf argument order is unspecified in C */
    printf("\ndelete \"anu\"     -> %d; search anu=%d, anusuya=%d, count_prefix(an)=%d, nodes=%ld (word was a prefix of anusuya: nodes kept)\n",
           d1, search(r, "anu"), search(r, "anusuya"), count_prefix(r, "an"), nodes_alive);
    int d2 = delete_word(r, "anusuya");
    printf("delete \"anusuya\" -> %d; count_prefix(an)=%d, nodes=%ld (branch pruned)\n", d2, count_prefix(r, "an"), nodes_alive);
    int d3 = delete_word(r, "zzz");
    printf("delete \"zzz\" (absent) -> %d\n", d3);
    destroy(r);

    puts("\n=== Load 5,000 names from data/names_5000.txt ===");
    FILE *f = fopen("data/names_5000.txt", "w");
    for (int i = 0; i < 5000; i++) { char nm[64]; make_name(nm); fprintf(f, "%s\n", nm); }
    fclose(f);
    f = fopen("data/names_5000.txt", "r"); r = node(); nodes_alive = 1; char line[64]; int loaded = 0;
    while (fgets(line, sizeof line, f)) { line[strcspn(line, "\n")] = 0; insert(r, line); loaded++; }
    fclose(f);
    printf("read %d lines, distinct words in trie = %d, nodes = %ld, memory = %.2f KB\n", loaded, count_prefix(r, ""), nodes_alive, nodes_alive * sizeof(Trie) / 1024.0);
    printf("prefix \"ka\": %d names, first 5 -> ", count_prefix(r, "ka")); autocomplete(r, "ka", 5, 0); puts("");
    printf("prefix \"mari\": %d names, first 5 -> ", count_prefix(r, "mari")); autocomplete(r, "mari", 5, 0); puts("");
    destroy(r);

    puts("\n=== Time per prefix search vs number of names (prefix length 3) ===");
    printf("%-8s %-12s %-16s %-18s %-16s\n", "names", "trie nodes", "trie memory", "sorted-array mem", "ns per count_prefix");
    int sizes[] = {500, 5000, 50000};
    for (int s = 0; s < 3; s++) {
        int n = sizes[s]; char **names = malloc(n * sizeof *names); size_t strbytes = 0;
        r = node(); nodes_alive = 1;
        for (int i = 0; i < n; i++) { char nm[64]; make_name(nm); names[i] = strdup(nm); insert(r, nm); strbytes += strlen(nm) + 1; }
        qsort(names, n, sizeof *names, cmp_str);
        const int Q = 200000; volatile long sink = 0; char q[4] = "ka";
        double t0 = now_s();
        for (int i = 0; i < Q; i++) { q[1] = 'a' + (i % 26); q[2] = 'a' + (i / 26 % 26); q[3] = 0; sink += count_prefix(r, q); }
        double dt = now_s() - t0;
        printf("%-8d %-12ld %10.1f KB %14.1f KB %14.0f\n", n, nodes_alive, nodes_alive * sizeof(Trie) / 1024.0, (n * sizeof(char *) + strbytes) / 1024.0, dt / Q * 1e9);
        for (int i = 0; i < n; i++) free(names[i]);
        free(names); destroy(r);
    }
    puts("(time is nearly flat as names grow x100: it depends on prefix LENGTH, not the number of words)");
    return 0;
}
