/* Q3 - B-tree (minimum degree t=2, i.e. order 4): insert with splits, search, disk-read analysis */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define T 2                                   /* minimum degree: node holds T-1..2T-1 keys */
typedef struct BNode {
    int n, leaf, keys[2 * T - 1];
    struct BNode *c[2 * T];
} BNode;

static int splits = 0;
static BNode *new_node(int leaf) { BNode *x = calloc(1, sizeof *x); x->leaf = leaf; return x; }

/* search: returns 1 if found; *visits = nodes read (= disk reads) */
static int search(BNode *x, int k, int *visits) {
    *visits = 0;
    while (x) {
        (*visits)++;
        int i = 0;
        while (i < x->n && k > x->keys[i]) i++;
        if (i < x->n && k == x->keys[i]) return 1;
        if (x->leaf) return 0;
        x = x->c[i];
    }
    return 0;
}
static void split_child(BNode *p, int i) {     /* p->c[i] is full: median moves up */
    BNode *y = p->c[i], *z = new_node(y->leaf);
    splits++;
    z->n = T - 1;
    for (int j = 0; j < T - 1; j++) z->keys[j] = y->keys[j + T];
    if (!y->leaf) for (int j = 0; j < T; j++) z->c[j] = y->c[j + T];
    y->n = T - 1;
    for (int j = p->n; j >= i + 1; j--) p->c[j + 1] = p->c[j];
    p->c[i + 1] = z;
    for (int j = p->n - 1; j >= i; j--) p->keys[j + 1] = p->keys[j];
    p->keys[i] = y->keys[T - 1];
    p->n++;
}
static void insert_nonfull(BNode *x, int k) {
    int i = x->n - 1;
    if (x->leaf) {
        while (i >= 0 && k < x->keys[i]) { x->keys[i + 1] = x->keys[i]; i--; }
        x->keys[i + 1] = k; x->n++;
    } else {
        while (i >= 0 && k < x->keys[i]) i--;
        i++;
        if (x->c[i]->n == 2 * T - 1) { split_child(x, i); if (k > x->keys[i]) i++; }
        insert_nonfull(x->c[i], k);
    }
}
static BNode *insert(BNode *root, int k) {
    if (!root) { root = new_node(1); root->keys[0] = k; root->n = 1; return root; }
    if (root->n == 2 * T - 1) {
        BNode *s = new_node(0); s->c[0] = root; split_child(s, 0); root = s;
    }
    insert_nonfull(root, k);
    return root;
}
static void print_level(BNode *x, int target, int depth) {
    if (!x) return;
    if (depth == target) { printf("["); for (int i = 0; i < x->n; i++) printf(i ? " %d" : "%d", x->keys[i]); printf("] "); return; }
    for (int i = 0; i <= x->n && !x->leaf; i++) print_level(x->c[i], target, depth + 1);
}
static int tree_height(BNode *x) { int h = 1; while (!x->leaf) { x = x->c[0]; h++; } return h; }
static void show(BNode *r) { for (int d = 0; d < tree_height(r); d++) { printf("   level %d: ", d); print_level(r, d, 0); puts(""); } }
static void destroy(BNode *x) { if (!x) return; if (!x->leaf) for (int i = 0; i <= x->n; i++) destroy(x->c[i]); free(x); }

int main(void) {
    puts("=== Insert 10,20,5,6,12,30,7,17 into B-tree (t=2) ===");
    int ks[] = {10, 20, 5, 6, 12, 30, 7, 17}; BNode *root = NULL;
    for (int i = 0; i < 8; i++) {
        int before = splits;
        root = insert(root, ks[i]);
        printf("insert %2d  (splits this step: %d)\n", ks[i], splits - before);
        show(root);
    }
    printf("total splits = %d, height = %d (all leaves at same depth)\n", splits, tree_height(root));

    puts("\n=== Search tests (node visits = disk reads) ===");
    int tests[] = {5, 30, 17, 99, 1}; const char *what[] = {"smallest", "largest", "middle", "missing (> max)", "missing (< min)"};
    for (int i = 0; i < 5; i++) { int v; int f = search(root, tests[i], &v); printf("search %3d (%-16s): found=%d, node visits=%d\n", tests[i], what[i], f, v); }
    destroy(root);

    puts("\n=== Larger B-tree: 100,000 random keys, t=2 vs the theory ===");
    root = NULL; srand(3);
    for (int i = 0; i < 100000; i++) root = insert(root, rand());
    printf("t=2 tree height = %d levels\n", tree_height(root));
    destroy(root);

    puts("\n=== Disk reads for 5,000,000 keys at 10 ms per read ===");
    double n = 5e6, ms = 10.0;
    int order = 100, tmin = (order + 1) / 2;          /* min children = ceil(m/2) = 50 */
    double bst_h = ceil(log2(n + 1)), avl_worst = 1.44 * log2(n + 2), avl_typ = log2(n) + 0.5;
    double bt_h_max = floor(log(((n + 1) / 2)) / log((double)tmin)) + 1, bt_h_min = ceil(log(n + 1) / log((double)order));
    printf("Order-100 B-tree: min children = %d\n", tmin);
    printf("  height bound  h <= log_%d((n+1)/2) + 1 = %.2f  -> worst case %d levels\n", tmin, log(((n + 1) / 2)) / log((double)tmin) + 1, (int)bt_h_max);
    printf("  best case (all nodes full): log_%d(n+1) = %.2f -> %d levels\n", order, log(n + 1) / log((double)order), (int)bt_h_min);
    printf("%-22s %12s %14s\n", "structure", "node visits", "time / search");
    printf("%-22s %12.0f %11.0f ms\n", "BST (balanced)", bst_h, bst_h * ms);
    printf("%-22s %12.0f %11.0f ms\n", "AVL (typical ~log2 n)", ceil(avl_typ), ceil(avl_typ) * ms);
    printf("%-22s %12.0f %11.0f ms\n", "AVL (worst 1.44 log2)", ceil(avl_worst), ceil(avl_worst) * ms);
    printf("%-22s %12d %11d ms\n", "B-tree order 100", (int)bt_h_max, (int)bt_h_max * 10);
    return 0;
}
