/* Q5 - Splay tree (zig / zig-zig / zig-zag) vs AVL on skewed and uniform access */
#include <stdio.h>
#include <stdlib.h>

typedef struct S { int key; struct S *l, *r; } S;
static int verbose = 0;
static S *rrot(S *y) { S *x = y->l; y->l = x->r; x->r = y; return x; }
static S *lrot(S *x) { S *y = x->r; x->r = y->l; y->l = x; return y; }

/* Recursive bottom-up splay: brings key (or the last node on its search path) to the root */
static S *splay(S *r, int k) {
    if (!r || r->key == k) return r;
    if (k < r->key) {
        if (!r->l) return r;
        if (k < r->l->key) {                         /* left-left  => ZIG-ZIG */
            r->l->l = splay(r->l->l, k);
            if (verbose) printf("   zig-zig at %d\n", r->key);
            r = rrot(r);
        } else if (k > r->l->key) {                  /* left-right => ZIG-ZAG */
            r->l->r = splay(r->l->r, k);
            if (verbose) printf("   zig-zag at %d\n", r->key);
            if (r->l->r) r->l = lrot(r->l);
        } else if (verbose) printf("   zig at %d\n", r->key);   /* child is the target => single ZIG */
        if (!r->l) return r;
        return rrot(r);                              /* final rotation (a ZIG, or 2nd half of the double) */
    } else {
        if (!r->r) return r;
        if (k > r->r->key) {                         /* right-right => ZIG-ZIG */
            r->r->r = splay(r->r->r, k);
            if (verbose) printf("   zig-zig at %d\n", r->key);
            r = lrot(r);
        } else if (k < r->r->key) {                  /* right-left => ZIG-ZAG */
            r->r->l = splay(r->r->l, k);
            if (verbose) printf("   zig-zag at %d\n", r->key);
            if (r->r->l) r->r = rrot(r->r);
        } else if (verbose) printf("   zig at %d\n", r->key);
        if (!r->r) return r;
        return lrot(r);
    }
}
static S *bst_insert(S *t, int k) {                  /* plain BST insert (no splaying) - to build an initial tree */
    S *n = calloc(1, sizeof *n); n->key = k;
    if (!t) return n;
    S *p = t;
    for (;;) { if (k < p->key) { if (!p->l) { p->l = n; break; } p = p->l; } else if (k > p->key) { if (!p->r) { p->r = n; break; } p = p->r; } else { free(n); break; } }
    return t;
}
static int path_len(S *t, int k) { int v = 0; while (t) { v++; if (k == t->key) break; t = k < t->key ? t->l : t->r; } return v; }
static S *access_splay(S *t, int k, int *visited) { *visited = path_len(t, k); return splay(t, k); }
static void show(S *t, int d) { if (!t) return; show(t->r, d + 1); printf("%*s%d\n", d * 5, "", t->key); show(t->l, d + 1); }
static void destroy(S *t) { if (!t) return; S **st = malloc(200000 * sizeof *st); int s = 0; st[s++] = t; while (s) { S *n = st[--s]; if (n->l) st[s++] = n->l; if (n->r) st[s++] = n->r; free(n); } free(st); }

/* ---- AVL (insert + search only) as the "always balanced" competitor ---- */
typedef struct A { int key, h; struct A *l, *r; } A;
static int ah(A *n) { return n ? n->h : 0; }
static int mx(int a, int b) { return a > b ? a : b; }
static void af(A *n) { n->h = 1 + mx(ah(n->l), ah(n->r)); }
static A *ar(A *y) { A *x = y->l; y->l = x->r; x->r = y; af(y); af(x); return x; }
static A *al(A *x) { A *y = x->r; x->r = y->l; y->l = x; af(x); af(y); return y; }
static A *avl_ins(A *n, int k) {
    if (!n) { n = malloc(sizeof *n); n->key = k; n->h = 1; n->l = n->r = NULL; return n; }
    if (k < n->key) n->l = avl_ins(n->l, k); else if (k > n->key) n->r = avl_ins(n->r, k); else return n;
    af(n); int b = ah(n->l) - ah(n->r);
    if (b > 1) { if (ah(n->l->l) < ah(n->l->r)) n->l = al(n->l); return ar(n); }
    if (b < -1) { if (ah(n->r->r) < ah(n->r->l)) n->r = ar(n->r); return al(n); }
    return n;
}
static int avl_visits(A *t, int k) { int v = 0; while (t) { v++; if (k == t->key) break; t = k < t->key ? t->l : t->r; } return v; }
static void avl_free(A *n) { if (n) { avl_free(n->l); avl_free(n->r); free(n); } }
static void shuffle(int *a, int n) { for (int i = n - 1; i > 0; i--) { int j = rand() % (i + 1); int t = a[i]; a[i] = a[j]; a[j] = t; } }

int main(void) {
    srand(5);
    puts("=== Trace: splay 3 then 9 in a BST built from 6,2,9,1,4,3,8,10,7 ===");
    int init[] = {6, 2, 9, 1, 4, 3, 8, 10, 7}; S *t = NULL;
    for (int i = 0; i < 9; i++) t = bst_insert(t, init[i]);
    puts("initial tree (root at left):"); show(t, 0);
    verbose = 1;
    printf("\naccess 3:\n"); t = splay(t, 3); show(t, 0);
    printf("\naccess 9:\n"); t = splay(t, 9); show(t, 0);
    verbose = 0; destroy(t);

    const int n = 1000, ops = 10000;
    int *keys = malloc(n * sizeof *keys); for (int i = 0; i < n; i++) keys[i] = i + 1; shuffle(keys, n);
    A *av = NULL; for (int i = 0; i < n; i++) av = avl_ins(av, keys[i]);
    const char *names[] = {"SKEWED 80% of accesses -> 5% of keys", "UNIFORM (every key equally likely)", "VERY SKEWED 95% of accesses -> 1% of keys"};
    int hots[] = {n / 20, 0, n / 100}, pcts[] = {80, 0, 95};
    puts("\n=== 10,000 accesses over 1,000 keys ===");
    for (int w = 0; w < 3; w++) {
        int hot = hots[w];                                   /* the first 'hot' shuffled keys are the popular ones */
        S *s2 = NULL; for (int i = 0; i < n; i++) s2 = bst_insert(s2, keys[i]);   /* fresh tree per workload */
        long sv = 0, av_v = 0; int worst = 0, v;
        for (int i = 0; i < ops; i++) {
            int k = (w != 1 && rand() % 100 < pcts[w]) ? keys[rand() % hot] : keys[rand() % n];
            s2 = access_splay(s2, k, &v); sv += v; if (v > worst) worst = v;
            av_v += avl_visits(av, k);
            if (w == 0 && i < 10) printf("   access #%d key %-4d visited %-3d -> root after splay = %d\n", i + 1, k, v, s2->key);
        }
        printf("%-42s splay avg %5.2f | AVL avg %5.2f | splay worst single access %d\n", names[w], (double)sv / ops, (double)av_v / ops, worst);
        destroy(s2);
    }

    puts("\n=== Sorted-order access: insert 1..1000 with splaying, then access 1..1000 in order ===");
    S *sorted = NULL; int v;
    for (int i = 1; i <= n; i++) { sorted = bst_insert(sorted, i); sorted = splay(sorted, i); }
    long tot = 0; int worst = 0;
    for (int i = 1; i <= n; i++) { sorted = access_splay(sorted, i, &v); tot += v; if (v > worst) worst = v; }
    printf("tree is a long spine after the inserts; first access visits %d nodes (worst single access = %d, an O(n) operation!)\n", worst, worst);
    printf("but total for all %d sequential accesses = %ld nodes (avg %.2f) -> amortised O(1) each\n", n, tot, (double)tot / n);
    destroy(sorted); avl_free(av); free(keys);
    return 0;
}
