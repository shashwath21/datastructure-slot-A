/* Q7 - Dictionary for a language app: exact look-up, spell-check suggestions, autocomplete,
 *      alphabetical range listing.  Design: TRIE with first-child / next-sibling lists
 *      (sorted siblings) instead of 26 pointers per node -> ~9x less memory, alphabetical order for free.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct TN { char c; unsigned char end; struct TN *child, *sib; } TN;   /* 24 bytes */
static long nodes = 0;

static TN *find_child(TN *n, char c) { for (TN *x = n->child; x; x = x->sib) { if (x->c == c) return x; if (x->c > c) break; } return NULL; }
static void insert(TN *root, const char *w) {
    TN *t = root;
    for (; *w; w++) {
        TN **pp = &t->child;                                   /* keep siblings sorted */
        while (*pp && (*pp)->c < *w) pp = &(*pp)->sib;
        if (!*pp || (*pp)->c != *w) {
            TN *n = calloc(1, sizeof *n); n->c = *w; n->sib = *pp; *pp = n; nodes++;
        }
        t = *pp;
    }
    t->end = 1;
}
static int lookup(TN *root, const char *w) {
    TN *t = root;
    for (; *w; w++) { t = find_child(t, *w); if (!t) return 0; }
    return t->end;
}
/* ---- autocomplete: all words with this prefix, alphabetical ---- */
static void walk(TN *n, char *buf, int d, int limit, int *cnt, int print) {
    for (TN *x = n; x && *cnt < limit; x = x->sib) {
        buf[d] = x->c; buf[d + 1] = 0;
        if (x->end) { if (print) printf("%s ", buf); (*cnt)++; if (*cnt >= limit) return; }
        walk(x->child, buf, d + 1, limit, cnt, print);
    }
}
static int autocomplete(TN *root, const char *prefix, int limit, int print) {
    TN *t = root;
    for (const char *p = prefix; *p; p++) { t = find_child(t, *p); if (!t) return 0; }
    char buf[128]; strcpy(buf, prefix); int cnt = 0;
    if (t != root && t->end) { if (print) printf("%s ", buf); cnt++; }
    walk(t->child, buf, (int)strlen(prefix), limit, &cnt, print);
    return cnt;
}
/* ---- range listing lo <= word <= hi with pruning of whole subtrees ---- */
static void range_rec(TN *n, char *buf, int d, const char *lo, const char *hi, int *cnt, int print) {
    for (TN *x = n; x; x = x->sib) {
        buf[d] = x->c; buf[d + 1] = 0;
        if (strncmp(buf, lo, d + 1) < 0) continue;            /* everything below is < lo */
        if (strncmp(buf, hi, d + 1) > 0) return;              /* everything below (and to the right) is > hi */
        if (x->end && strcmp(buf, lo) >= 0 && strcmp(buf, hi) <= 0) { if (print) printf("%s ", buf); (*cnt)++; }
        range_rec(x->child, buf, d + 1, lo, hi, cnt, print);
    }
}
static int range(TN *root, const char *lo, const char *hi, int print) { char buf[128]; int cnt = 0; range_rec(root->child, buf, 0, lo, hi, &cnt, print); return cnt; }
/* ---- spell-check: all dictionary words within edit distance 1 (delete / transpose / replace / insert) ---- */
static int cmp_str(const void *a, const void *b) { return strcmp(*(char *const *)a, *(char *const *)b); }
static int suggest(TN *root, const char *w, char out[][32], int max) {
    int n = 0, len = (int)strlen(w); char c[40];
    #define TRY() do { if (lookup(root, c)) { int dup = 0; for (int q = 0; q < n; q++) if (!strcmp(out[q], c)) dup = 1; if (!dup && n < max) strcpy(out[n++], c); } } while (0)
    for (int i = 0; i < len; i++) { memcpy(c, w, i); strcpy(c + i, w + i + 1); TRY(); }                          /* deletion */
    for (int i = 0; i + 1 < len; i++) { strcpy(c, w); c[i] = w[i + 1]; c[i + 1] = w[i]; TRY(); }                   /* transposition */
    for (int i = 0; i < len; i++) for (char ch = 'a'; ch <= 'z'; ch++) { strcpy(c, w); c[i] = ch; TRY(); }          /* replacement */
    for (int i = 0; i <= len; i++) for (char ch = 'a'; ch <= 'z'; ch++) { memcpy(c, w, i); c[i] = ch; strcpy(c + i + 1, w + i); TRY(); }  /* insertion */
    for (int i = 0; i < n; i++) for (int j = i + 1; j < n; j++) if (strcmp(out[i], out[j]) > 0) { char t[32]; strcpy(t, out[i]); strcpy(out[i], out[j]); strcpy(out[j], t); }
    return n;
}
static void destroy(TN *n) { while (n) { TN *s = n->sib; destroy(n->child); free(n); n = s; } }

/* ---- sample data ---- */
static const char *SYL[] = {"ka","ma","ni","ra","tha","vi","su","lo","pa","mi","ven","dha","na","la","ti","re","ga","mu","sai","kan"};
static char *mk(void) { char b[40] = ""; int s = 2 + rand() % 4; for (int i = 0; i < s; i++) strcat(b, SYL[rand() % 20]); return strdup(b); }
static char **gen_unique(int n, int *outn) {
    char **w = malloc(n * 3 * sizeof *w); int m = 0;
    for (int i = 0; i < n * 3; i++) w[m++] = mk();
    qsort(w, m, sizeof *w, cmp_str);
    int u = 0; for (int i = 0; i < m; i++) { if (u && !strcmp(w[u - 1], w[i])) free(w[i]); else w[u++] = w[i]; }
    if (u > n) {                                              /* random subset (not the alphabetical head), then re-sort */
        for (int i = u - 1; i > 0; i--) { int j = rand() % (i + 1); char *t = w[i]; w[i] = w[j]; w[j] = t; }
        for (int i = n; i < u; i++) free(w[i]);
        u = n; qsort(w, u, sizeof *w, cmp_str);
    }
    *outn = u; return w;
}
static double now_s(void) { struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); return ts.tv_sec + ts.tv_nsec * 1e-9; }
static int lower_bound(char **a, int n, const char *k) { int lo = 0, hi = n; while (lo < hi) { int m = (lo + hi) / 2; if (strcmp(a[m], k) < 0) lo = m + 1; else hi = m; } return lo; }

int main(void) {
    srand(99);
    puts("=== STAGE 3: verify on a 1,000-word sample (trie result vs sorted-array reference) ===");
    int n; char **w = gen_unique(1000, &n);                         /* sorted, unique */
    FILE *f = fopen("data/dictionary_1000.txt", "w"); for (int i = 0; i < n; i++) fprintf(f, "%s\n", w[i]); fclose(f);
    TN root = {0}; nodes = 0; for (int i = 0; i < n; i++) insert(&root, w[i]);
    printf("words = %d, trie nodes = %ld\n", n, nodes);

    int ok = 0; for (int i = 0; i < n; i++) ok += lookup(&root, w[i]);
    printf("exact look-up of all %d words: %d found %s\n", n, ok, ok == n ? "PASS" : "FAIL");
    int bad = 0, tested = 0;
    for (int i = 0; i < 2000; i++) { char *x = mk(); int ref = lower_bound(w, n, x) < n && !strcmp(w[lower_bound(w, n, x)], x); tested++; if (lookup(&root, x) != ref) bad++; free(x); }
    printf("2000 random probe words (present or absent) vs reference: %d mismatches %s\n", bad, bad == 0 ? "PASS" : "FAIL");

    const char *pfx[] = {"ka", "mani", "thav", "sai", "zz", "ven"}; bad = 0;
    for (int i = 0; i < 6; i++) {
        int got = autocomplete(&root, pfx[i], 100000, 0), ref = 0;
        for (int j = 0; j < n; j++) if (!strncmp(w[j], pfx[i], strlen(pfx[i]))) ref++;
        if (got != ref) bad++;
        printf("  autocomplete \"%s\": %d words (reference %d)\n", pfx[i], got, ref);
    }
    printf("autocomplete counts: %s\n", bad == 0 ? "PASS" : "FAIL");
    printf("  first 5 for \"ka\": "); autocomplete(&root, "ka", 5, 1); puts("");

    const char *lo[] = {"ka", "mani", "a", "tha"}, *hi[] = {"kz", "mz", "zzz", "thb"}; bad = 0;
    for (int i = 0; i < 4; i++) {
        int got = range(&root, lo[i], hi[i], 0), ref = 0;
        for (int j = 0; j < n; j++) if (strcmp(w[j], lo[i]) >= 0 && strcmp(w[j], hi[i]) <= 0) ref++;
        if (got != ref) bad++;
        printf("  range [%s .. %s]: %d words (reference %d)\n", lo[i], hi[i], got, ref);
    }
    printf("range listing counts: %s\n", bad == 0 ? "PASS" : "FAIL");
    printf("  range [kana .. kani] -> "); range(&root, "kana", "kani", 1); puts("");

    char sg[64][32]; char typo[40]; strcpy(typo, w[100]); typo[1] = (typo[1] == 'z') ? 'y' : 'z';      /* damage word #100 */
    int ns = suggest(&root, typo, sg, 64);
    printf("spell-check: \"%s\" (typo of \"%s\") -> suggestions:", typo, w[100]); for (int i = 0; i < ns; i++) printf(" %s", sg[i]); puts("");

    puts("\n=== Memory and response time at 1,000 / 20,000 / 200,000 words ===");
    printf("%-9s %-11s %-18s %-19s %-18s %-14s %-14s\n", "words", "nodes", "sibling-trie", "26-pointer trie", "sorted array", "lookup", "autocomplete(3)");
    int sizes[] = {1000, 20000, 200000};
    for (int s = 0; s < 3; s++) {
        int m; char **ws = gen_unique(sizes[s], &m);
        TN r2 = {0}; nodes = 0; size_t strb = 0;
        for (int i = 0; i < m; i++) { insert(&r2, ws[i]); strb += strlen(ws[i]) + 1; }
        int Q = 200000; volatile int sink = 0; double t0 = now_s();
        for (int i = 0; i < Q; i++) sink += lookup(&r2, ws[(i * 7919) % m]);
        double t_look = (now_s() - t0) / Q * 1e9;
        char pre[4]; t0 = now_s(); int QA = 20000;
        for (int i = 0; i < QA; i++) { const char *src = ws[(i * 104729) % m]; pre[0] = src[0]; pre[1] = src[1]; pre[2] = src[2]; pre[3] = 0; sink += autocomplete(&r2, pre, 10, 0); }
        double t_auto = (now_s() - t0) / QA * 1e6;
        printf("%-9d %-11ld %10.2f MB      %10.2f MB       %10.2f MB      %7.0f ns      %7.2f us\n", m, nodes,
               nodes * sizeof(TN) / 1048576.0, (nodes + 1) * 216 / 1048576.0, (m * sizeof(char *) + strb) / 1048576.0, t_look, t_auto);
        for (int i = 0; i < m; i++) free(ws[i]);
        free(ws); destroy(r2.child);
    }
    for (int i = 0; i < n; i++) free(w[i]);
    free(w); destroy(root.child);
    return 0;
}
