/* Q2 - AVL vs BST: step-by-step insertions, rotation counts, timing, balance verification */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ---------- AVL ---------- */
typedef struct A { int key, h; struct A *l, *r; } A;
static long rot_count = 0;
static const char *last_rot = "none";
static int ht(A *n) { return n ? n->h : 0; }
static int mx(int a, int b) { return a > b ? a : b; }
static void fix(A *n) { n->h = 1 + mx(ht(n->l), ht(n->r)); }
static A *rrot(A *y) { A *x = y->l; y->l = x->r; x->r = y; fix(y); fix(x); rot_count++; return x; }
static A *lrot(A *x) { A *y = x->r; x->r = y->l; y->l = x; fix(x); fix(y); rot_count++; return y; }
static A *avl_insert(A *n, int key) {
    if (!n) { n = malloc(sizeof *n); n->key = key; n->h = 1; n->l = n->r = NULL; return n; }
    if (key < n->key) n->l = avl_insert(n->l, key);
    else if (key > n->key) n->r = avl_insert(n->r, key);
    else return n;
    fix(n);
    int bal = ht(n->l) - ht(n->r);
    if (bal > 1 && key < n->l->key)  { last_rot = "LL (single right)"; return rrot(n); }
    if (bal < -1 && key > n->r->key) { last_rot = "RR (single left)";  return lrot(n); }
    if (bal > 1)  { last_rot = "LR (left-right)"; n->l = lrot(n->l); return rrot(n); }
    if (bal < -1) { last_rot = "RL (right-left)"; n->r = rrot(n->r); return lrot(n); }
    return n;
}
static int avl_search(A *n, int k) { while (n) { if (k == n->key) return 1; n = k < n->key ? n->l : n->r; } return 0; }
static int verify(A *n) {                         /* checks |balance|<=1 and stored heights */
    if (!n) return 1;
    int b = ht(n->l) - ht(n->r);
    if (b < -1 || b > 1) return 0;
    if (n->h != 1 + mx(ht(n->l), ht(n->r))) return 0;
    return verify(n->l) && verify(n->r);
}
static void avl_free(A *n) { if (n) { avl_free(n->l); avl_free(n->r); free(n); } }
static void show(A *n, int depth) {               /* sideways picture: root at left, right subtree on top */
    if (!n) return;
    show(n->r, depth + 1);
    printf("%*s%d (h=%d,bf=%d)\n", depth * 6, "", n->key, n->h, ht(n->l) - ht(n->r));
    show(n->l, depth + 1);
}

/* ---------- plain BST (iterative, so 100,000 sorted keys cannot overflow the stack) ---------- */
typedef struct B { int key; struct B *l, *r; } B;
static B *bst_insert(B *root, int key) {
    B *n = calloc(1, sizeof *n); n->key = key;
    if (!root) return n;
    B *p = root;
    for (;;) {
        if (key < p->key) { if (!p->l) { p->l = n; break; } p = p->l; }
        else if (key > p->key) { if (!p->r) { p->r = n; break; } p = p->r; }
        else { free(n); break; }
    }
    return root;
}
static int bst_search(B *n, int k) { while (n) { if (k == n->key) return 1; n = k < n->key ? n->l : n->r; } return 0; }
static int bst_height(B *root) {                  /* iterative level count */
    if (!root) return 0;
    B **q = malloc(200000 * sizeof *q); int h = 0, e = 0, lv = 0;
    q[e++] = root;
    while (h < e) { int sz = e - h; lv++; while (sz--) { B *n = q[h++]; if (n->l) q[e++] = n->l; if (n->r) q[e++] = n->r; } }
    free(q); return lv;
}
static void bst_free(B *r) { if (!r) return; B **st = malloc(200000 * sizeof *st); int s = 0; st[s++] = r;
    while (s) { B *n = st[--s]; if (n->l) st[s++] = n->l; if (n->r) st[s++] = n->r; free(n); } free(st); }

static double now(void) { return (double)clock() / CLOCKS_PER_SEC; }
static void shuffle(int *a, int n) { for (int i = n - 1; i > 0; i--) { int j = rand() % (i + 1); int t = a[i]; a[i] = a[j]; a[j] = t; } }

int main(void) {
    srand(7);
    puts("=== Insert 10,20,30,40,50,25 one by one (AVL) ===");
    int p[] = {10, 20, 30, 40, 50, 25}; A *r = NULL;
    for (int i = 0; i < 6; i++) {
        last_rot = "none"; long before = rot_count;
        r = avl_insert(r, p[i]);
        printf("\n-- after inserting %d: rotation = %s (%ld rotation step(s)), height = %d, balanced = %d\n",
               p[i], rot_count == before ? "none" : last_rot, rot_count - before, ht(r), verify(r));
        show(r, 0);
    }
    avl_free(r);

    puts("\n=== 100,000 SORTED keys: BST vs AVL ===");
    int n = 100000, *k = malloc(n * sizeof *k);
    for (int i = 0; i < n; i++) k[i] = i + 1;
    double t0 = now(); B *b = NULL; for (int i = 0; i < n; i++) b = bst_insert(b, k[i]);
    double bst_ins = now() - t0;
    rot_count = 0; t0 = now(); A *a = NULL; for (int i = 0; i < n; i++) a = avl_insert(a, k[i]);
    double avl_ins = now() - t0; long avl_rot_sorted = rot_count;
    /* search 2000 keys, 100 passes - BST sorted tree is O(n) per search so use fewer */
    t0 = now(); long hit = 0; for (int i = 0; i < 2000; i++) hit += bst_search(b, k[n - 1 - i * 5]); double bst_srch = now() - t0;
    t0 = now(); for (int rep = 0; rep < 1; rep++) for (int i = 0; i < 2000; i++) hit += avl_search(a, k[n - 1 - i * 5]); double avl_srch = now() - t0;
    printf("BST height = %d\nAVL height = %d   (1.44*log2(n) bound = %.1f)\n", bst_height(b), ht(a), 1.44 * 16.61);
    printf("insert time: BST %.4f s | AVL %.4f s\n", bst_ins, avl_ins);
    printf("2000 searches: BST %.4f s | AVL %.6f s   (hits=%ld)\n", bst_srch, avl_srch, hit);
    printf("AVL rotations for sorted insertions: %ld\n", avl_rot_sorted);
    printf("balance factor of every node within [-1,1] and heights correct: %s\n", verify(a) ? "VERIFIED" : "FAILED");
    bst_free(b); avl_free(a);

    puts("\n=== 100,000 RANDOM keys ===");
    shuffle(k, n);
    t0 = now(); b = NULL; for (int i = 0; i < n; i++) b = bst_insert(b, k[i]); bst_ins = now() - t0;
    rot_count = 0; t0 = now(); a = NULL; for (int i = 0; i < n; i++) a = avl_insert(a, k[i]); avl_ins = now() - t0;
    t0 = now(); for (int i = 0; i < n; i++) hit += bst_search(b, k[i]); bst_srch = now() - t0;
    t0 = now(); for (int i = 0; i < n; i++) hit += avl_search(a, k[i]); avl_srch = now() - t0;
    printf("BST height = %d | AVL height = %d\n", bst_height(b), ht(a));
    printf("insert time: BST %.4f s | AVL %.4f s\n", bst_ins, avl_ins);
    printf("100000 searches: BST %.4f s | AVL %.4f s\n", bst_srch, avl_srch);
    printf("AVL rotations for random insertions: %ld (%.2f per insertion)\n", rot_count, (double)rot_count / n);
    printf("balance verified: %s\n", verify(a) ? "VERIFIED" : "FAILED");
    bst_free(b); avl_free(a); free(k);
    return 0;
}
