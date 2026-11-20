/* Q4 - Red-black tree (CLRS) with insert/delete, property checker, and comparison with AVL */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================= RED-BLACK TREE ================= */
enum { RED = 0, BLACK = 1 };
typedef struct R { int key, color; struct R *l, *r, *p; } R;
static R NIL_NODE = { 0, BLACK, &NIL_NODE, &NIL_NODE, &NIL_NODE };
#define NIL (&NIL_NODE)
static R *root = NIL;
static long rb_rot = 0, rb_recolor = 0;
static int trace = 0;

static void left_rotate(R *x) {
    R *y = x->r; x->r = y->l; if (y->l != NIL) y->l->p = x;
    y->p = x->p;
    if (x->p == NIL) root = y; else if (x == x->p->l) x->p->l = y; else x->p->r = y;
    y->l = x; x->p = y; rb_rot++;
    if (trace) printf("      rotate LEFT at %d\n", x->key);
}
static void right_rotate(R *x) {
    R *y = x->l; x->l = y->r; if (y->r != NIL) y->r->p = x;
    y->p = x->p;
    if (x->p == NIL) root = y; else if (x == x->p->r) x->p->r = y; else x->p->l = y;
    y->r = x; x->p = y; rb_rot++;
    if (trace) printf("      rotate RIGHT at %d\n", x->key);
}
static void insert_fix(R *z) {
    while (z->p->color == RED) {
        if (z->p == z->p->p->l) {
            R *u = z->p->p->r;
            if (u->color == RED) {                         /* case 1: recolour */
                z->p->color = BLACK; u->color = BLACK; z->p->p->color = RED; rb_recolor++;
                if (trace) printf("      recolour: parent %d & uncle %d -> BLACK, grandparent %d -> RED\n", z->p->key, u->key, z->p->p->key);
                z = z->p->p;
            } else {
                if (z == z->p->r) { z = z->p; left_rotate(z); }    /* case 2 -> 3 */
                z->p->color = BLACK; z->p->p->color = RED; right_rotate(z->p->p);
            }
        } else {
            R *u = z->p->p->l;
            if (u->color == RED) {
                z->p->color = BLACK; u->color = BLACK; z->p->p->color = RED; rb_recolor++;
                if (trace) printf("      recolour: parent %d & uncle %d -> BLACK, grandparent %d -> RED\n", z->p->key, u->key, z->p->p->key);
                z = z->p->p;
            } else {
                if (z == z->p->l) { z = z->p; right_rotate(z); }
                z->p->color = BLACK; z->p->p->color = RED; left_rotate(z->p->p);
            }
        }
    }
    root->color = BLACK;
}
static void rb_insert(int key) {
    R *y = NIL, *x = root;
    while (x != NIL) { y = x; if (key == x->key) return; x = key < x->key ? x->l : x->r; }
    R *z = malloc(sizeof *z); z->key = key; z->color = RED; z->l = z->r = NIL; z->p = y;
    if (y == NIL) root = z; else if (key < y->key) y->l = z; else y->r = z;
    insert_fix(z);
}
static void transplant(R *u, R *v) { if (u->p == NIL) root = v; else if (u == u->p->l) u->p->l = v; else u->p->r = v; v->p = u->p; }
static R *tree_min(R *x) { while (x->l != NIL) x = x->l; return x; }
static void delete_fix(R *x) {
    while (x != root && x->color == BLACK) {
        if (x == x->p->l) {
            R *w = x->p->r;
            if (w->color == RED) { w->color = BLACK; x->p->color = RED; left_rotate(x->p); w = x->p->r; }
            if (w->l->color == BLACK && w->r->color == BLACK) { w->color = RED; rb_recolor++; x = x->p; }
            else {
                if (w->r->color == BLACK) { w->l->color = BLACK; w->color = RED; right_rotate(w); w = x->p->r; }
                w->color = x->p->color; x->p->color = BLACK; w->r->color = BLACK; left_rotate(x->p); x = root;
            }
        } else {
            R *w = x->p->l;
            if (w->color == RED) { w->color = BLACK; x->p->color = RED; right_rotate(x->p); w = x->p->l; }
            if (w->r->color == BLACK && w->l->color == BLACK) { w->color = RED; rb_recolor++; x = x->p; }
            else {
                if (w->l->color == BLACK) { w->r->color = BLACK; w->color = RED; left_rotate(w); w = x->p->l; }
                w->color = x->p->color; x->p->color = BLACK; w->l->color = BLACK; right_rotate(x->p); x = root;
            }
        }
    }
    x->color = BLACK;
}
static void rb_delete(int key) {
    R *z = root; while (z != NIL && z->key != key) z = key < z->key ? z->l : z->r;
    if (z == NIL) return;
    R *y = z, *x; int ycol = y->color;
    if (z->l == NIL) { x = z->r; transplant(z, z->r); }
    else if (z->r == NIL) { x = z->l; transplant(z, z->l); }
    else {
        y = tree_min(z->r); ycol = y->color; x = y->r;
        if (y->p == z) x->p = y; else { transplant(y, y->r); y->r = z->r; y->r->p = y; }
        transplant(z, y); y->l = z->l; y->l->p = y; y->color = z->color;
    }
    free(z);
    if (ycol == BLACK) delete_fix(x);
}
/* ---- checker for the five properties ---- */
static int check_node(R *x, int *ok) {                   /* returns black-height */
    if (x == NIL) return 1;                              /* P3: NIL leaves are black */
    if (x->color == RED && (x->l->color == RED || x->r->color == RED)) *ok = 0;      /* P4 */
    if (x->l != NIL && x->l->key >= x->key) *ok = 0;     /* BST order */
    if (x->r != NIL && x->r->key <= x->key) *ok = 0;
    int a = check_node(x->l, ok), b = check_node(x->r, ok);
    if (a != b) *ok = 0;                                 /* P5: equal black count on every path */
    return a + (x->color == BLACK);
}
static int rb_valid(void) { int ok = 1; if (root != NIL && root->color != BLACK) ok = 0; /* P2 */ check_node(root, &ok); return ok; }
static int rb_height(R *x) { if (x == NIL) return 0; int a = rb_height(x->l), b = rb_height(x->r); return 1 + (a > b ? a : b); }
static void rb_show(R *x, int d) { if (x == NIL) return; rb_show(x->r, d + 1); printf("%*s%d%c\n", d * 6, "", x->key, x->color == RED ? 'R' : 'B'); rb_show(x->l, d + 1); }
static void rb_free(R *x) { if (x == NIL) return; rb_free(x->l); rb_free(x->r); free(x); }

/* ================= AVL TREE (insert + delete) ================= */
typedef struct A { int key, h; struct A *l, *r; } A;
static long avl_rot = 0;
static int ht(A *n) { return n ? n->h : 0; }
static int mx(int a, int b) { return a > b ? a : b; }
static void fix(A *n) { n->h = 1 + mx(ht(n->l), ht(n->r)); }
static A *rr(A *y) { A *x = y->l; y->l = x->r; x->r = y; fix(y); fix(x); avl_rot++; return x; }
static A *lr(A *x) { A *y = x->r; x->r = y->l; y->l = x; fix(x); fix(y); avl_rot++; return y; }
static A *rebalance(A *n) {
    fix(n); int b = ht(n->l) - ht(n->r);
    if (b > 1)  { if (ht(n->l->l) < ht(n->l->r)) n->l = lr(n->l); return rr(n); }
    if (b < -1) { if (ht(n->r->r) < ht(n->r->l)) n->r = rr(n->r); return lr(n); }
    return n;
}
static A *avl_insert(A *n, int k) {
    if (!n) { n = malloc(sizeof *n); n->key = k; n->h = 1; n->l = n->r = NULL; return n; }
    if (k < n->key) n->l = avl_insert(n->l, k); else if (k > n->key) n->r = avl_insert(n->r, k); else return n;
    return rebalance(n);
}
static A *avl_delete(A *n, int k) {
    if (!n) return NULL;
    if (k < n->key) n->l = avl_delete(n->l, k);
    else if (k > n->key) n->r = avl_delete(n->r, k);
    else if (n->l && n->r) { A *s = n->r; while (s->l) s = s->l; n->key = s->key; n->r = avl_delete(n->r, s->key); }
    else { A *c = n->l ? n->l : n->r; free(n); return c; }
    return rebalance(n);
}
static void avl_free(A *n) { if (n) { avl_free(n->l); avl_free(n->r); free(n); } }

static void shuffle(int *a, int n) { for (int i = n - 1; i > 0; i--) { int j = rand() % (i + 1); int t = a[i]; a[i] = a[j]; a[j] = t; } }

int main(void) {
    srand(11);
    puts("The five red-black properties:");
    puts(" 1. Every node is red or black.        2. The root is black.");
    puts(" 3. Every NIL leaf is black.           4. A red node has only black children.");
    puts(" 5. All root-to-NIL paths contain the same number of black nodes.\n");

    puts("=== Insert 10,20,30,15,25,5 by hand-trace (R = red, B = black; root at left) ===");
    int ks[] = {10, 20, 30, 15, 25, 5}; trace = 1;
    for (int i = 0; i < 6; i++) {
        printf("\ninsert %d:\n", ks[i]); rb_insert(ks[i]); rb_show(root, 0);
        printf("   properties hold: %s\n", rb_valid() ? "yes" : "NO");
    }
    trace = 0;
    printf("\nSmallest key (leftmost node) = %d  (a scheduler caches this pointer: O(1) pick-next)\n", tree_min(root)->key);
    rb_free(root); root = NIL;

    puts("\n=== 10,000 random insertions then 10,000 random deletions: rotations ===");
    int n = 10000, *k = malloc(n * sizeof *k); for (int i = 0; i < n; i++) k[i] = i + 1; shuffle(k, n);
    int bad = 0; rb_rot = rb_recolor = 0; avl_rot = 0; A *a = NULL;
    for (int i = 0; i < n; i++) { rb_insert(k[i]); if (!rb_valid()) bad++; a = avl_insert(a, k[i]); }
    long rb_ins_rot = rb_rot, avl_ins_rot = avl_rot; int rbh = rb_height(root), avlh = ht(a);
    printf("after insertions : RB height %d | AVL height %d (limit 2*log2(n+1) = %.1f)\n", rbh, avlh, 2 * 13.29);
    shuffle(k, n); rb_rot = 0; avl_rot = 0;
    for (int i = 0; i < n; i++) { rb_delete(k[i]); if (!rb_valid()) bad++; a = avl_delete(a, k[i]); }
    long rb_del_rot = rb_rot, avl_del_rot = avl_rot;
    printf("\n%-22s %12s %12s\n", "rotations", "Red-black", "AVL");
    printf("%-22s %12ld %12ld   (%.2f vs %.2f per op)\n", "10,000 insertions", rb_ins_rot, avl_ins_rot, (double)rb_ins_rot / n, (double)avl_ins_rot / n);
    printf("%-22s %12ld %12ld   (%.2f vs %.2f per op)\n", "10,000 deletions", rb_del_rot, avl_del_rot, (double)rb_del_rot / n, (double)avl_del_rot / n);
    printf("RB recolourings during insertion+deletion: %ld\n", rb_recolor);
    printf("checker run after EVERY one of the 20,000 operations: %s\n", bad == 0 ? "all valid" : "VIOLATIONS FOUND");
    avl_free(a); free(k);

    puts("\n=== Increasing-order insertion 1..10000 ===");
    rb_rot = 0; avl_rot = 0; a = NULL; bad = 0;
    for (int i = 1; i <= 10000; i++) { rb_insert(i); a = avl_insert(a, i); }
    printf("RB height %d, AVL height %d, RB rotations %ld, AVL rotations %ld, RB valid: %s\n",
           rb_height(root), ht(a), rb_rot, avl_rot, rb_valid() ? "yes" : "NO");
    rb_free(root); root = NIL; avl_free(a);
    return 0;
}
