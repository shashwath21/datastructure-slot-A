/* Q1 - Full BST: insert/search/delete/height, 4 traversals, experiments, save & rebuild */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node { int key; struct Node *l, *r; } Node;

Node *insert(Node *t, int k) {
    if (!t) { Node *n = calloc(1, sizeof *n); n->key = k; return n; }
    if (k < t->key) t->l = insert(t->l, k);
    else if (k > t->key) t->r = insert(t->r, k);       /* duplicates ignored */
    return t;
}
int search(const Node *t, int k, int *cmp) {            /* counts comparisons */
    *cmp = 0;
    while (t) { (*cmp)++; if (k == t->key) return 1; t = k < t->key ? t->l : t->r; }
    return 0;
}
int height(const Node *t) { if (!t) return -1; int a = height(t->l), b = height(t->r); return 1 + (a > b ? a : b); }
Node *min_node(Node *t) { while (t && t->l) t = t->l; return t; }
Node *del(Node *t, int k) {
    if (!t) return NULL;
    if (k < t->key) t->l = del(t->l, k);
    else if (k > t->key) t->r = del(t->r, k);
    else if (t->l && t->r) {                            /* case 3: two children */
        Node *s = min_node(t->r);                       /* inorder successor */
        t->key = s->key;
        t->r = del(t->r, s->key);
    } else {                                            /* case 1 (leaf) and case 2 (one child) */
        Node *c = t->l ? t->l : t->r;
        free(t);
        return c;
    }
    return t;
}
void preorder(const Node *t, FILE *f)  { if (t) { fprintf(f, "%d ", t->key); preorder(t->l, f); preorder(t->r, f); } }
void inorder(const Node *t, FILE *f)   { if (t) { inorder(t->l, f); fprintf(f, "%d ", t->key); inorder(t->r, f); } }
void postorder(const Node *t, FILE *f) { if (t) { postorder(t->l, f); postorder(t->r, f); fprintf(f, "%d ", t->key); } }
void levelorder(const Node *t, FILE *f) {
    if (!t) return;
    const Node **q = malloc(100000 * sizeof *q); int h = 0, e = 0;
    q[e++] = t;
    while (h < e) { const Node *n = q[h++]; fprintf(f, "%d ", n->key); if (n->l) q[e++] = n->l; if (n->r) q[e++] = n->r; }
    free(q);
}
void destroy(Node *t) { if (t) { destroy(t->l); destroy(t->r); free(t); } }
int count(const Node *t) { return t ? 1 + count(t->l) + count(t->r) : 0; }
int is_bst(const Node *t, long lo, long hi) { return !t || (t->key > lo && t->key < hi && is_bst(t->l, lo, t->key) && is_bst(t->r, t->key, hi)); }

/* Save with PREORDER: reinserting keys in preorder rebuilds the identical shape
   (parent always precedes its children). Inorder would rebuild a degenerate list. */
void save_tree(const Node *t, const char *path) { FILE *f = fopen(path, "w"); preorder(t, f); fclose(f); }
Node *load_tree(const char *path) {
    FILE *f = fopen(path, "r"); Node *t = NULL; int k;
    if (!f) return NULL;
    while (fscanf(f, "%d", &k) == 1) t = insert(t, k);
    fclose(f); return t;
}
int same_shape(const Node *a, const Node *b) {
    if (!a || !b) return a == b;
    return a->key == b->key && same_shape(a->l, b->l) && same_shape(a->r, b->r);
}

static void shuffle(int *a, int n) { for (int i = n - 1; i > 0; i--) { int j = rand() % (i + 1); int t = a[i]; a[i] = a[j]; a[j] = t; } }
static void experiment(const char *label, const int *keys, int n) {
    Node *t = NULL; for (int i = 0; i < n; i++) t = insert(t, keys[i]);
    long total = 0; int c;
    for (int i = 0; i < n; i++) { search(t, keys[i], &c); total += c; }
    printf("%-14s n=%d  height=%4d  avg comparisons=%8.2f\n", label, n, height(t), (double)total / n);
    destroy(t);
}

int main(void) {
    srand(42);
    puts("=== Traversals on a small BST ===");
    int a[] = {50, 30, 70, 20, 40, 60, 80, 65};
    Node *t = NULL; for (int i = 0; i < 8; i++) t = insert(t, a[i]);
    printf("preorder : "); preorder(t, stdout);   puts("");
    printf("inorder  : "); inorder(t, stdout);    puts("   <- sorted");
    printf("postorder: "); postorder(t, stdout);  puts("");
    printf("level    : "); levelorder(t, stdout); puts("");
    printf("height = %d\n", height(t));

    puts("\n=== Deletion: leaf, one child, two children ===");
    t = del(t, 20); printf("delete 20 (leaf)         -> inorder: "); inorder(t, stdout); printf(" preorder: "); preorder(t, stdout); printf(" valid=%d\n", is_bst(t, -(1L<<40), 1L<<40));
    t = del(t, 30); printf("delete 30 (one child)    -> inorder: "); inorder(t, stdout); printf(" preorder: "); preorder(t, stdout); printf(" valid=%d\n", is_bst(t, -(1L<<40), 1L<<40));
    t = del(t, 50); printf("delete 50 (two children) -> inorder: "); inorder(t, stdout); printf(" preorder: "); preorder(t, stdout); printf(" valid=%d\n", is_bst(t, -(1L<<40), 1L<<40));

    puts("\n=== Save (preorder) to file and rebuild ===");
    save_tree(t, "outputs/tree_saved.txt");
    Node *u = load_tree("outputs/tree_saved.txt");
    printf("rebuilt tree has identical shape: %s\n", same_shape(t, u) ? "YES" : "NO");
    destroy(u); destroy(t);

    puts("\n=== Edge cases ===");
    Node *e = NULL; int c;
    printf("empty tree: search=%d height=%d count=%d\n", search(e, 5, &c), height(e), count(e));
    e = del(e, 5); printf("delete from empty tree OK (root=%p)\n", (void *)e);
    for (int i = 0; i < 3; i++) e = insert(e, 7);
    e = insert(e, 3); e = insert(e, 3);
    printf("inserted 7,7,7,3,3 -> count=%d (duplicates ignored) inorder: ", count(e)); inorder(e, stdout); puts("");
    destroy(e);

    puts("\n=== Experiment: 2000 keys, random vs sorted insertion ===");
    int n = 2000, *keys = malloc(n * sizeof *keys);
    for (int i = 0; i < n; i++) keys[i] = i + 1;
    experiment("sorted order", keys, n);
    shuffle(keys, n);
    experiment("random order", keys, n);
    printf("ideal (perfectly balanced) height = floor(log2(2000)) = 10\n");
    free(keys);
    return 0;
}
