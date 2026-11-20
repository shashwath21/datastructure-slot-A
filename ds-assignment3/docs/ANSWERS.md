# Written answers (notes for the report)

Numbers below come from one run of `make run`; timings vary by machine, so re-quote them from your own `outputs/`.

---
## Q1 – Binary Search Trees

**Predict the output #1** – array `"ABCDE.F"`: A is the root, B and C its children, D and E are B's children, C's left child (index 5) is empty and F (index 6) is C's right child.

```
        A
      /   \
     B     C
    / \     \
   D   E     F
```
Output: `ABDECF DBEACF DEBFCA ABCDEF` (preorder, inorder, postorder, level order). The last loop walks the array from index 0 to N-1; because the array stores the tree level by level, that is a **level-order (breadth-first)** traversal. No difference from the prediction is expected – except that the Word file prints `││` which is not valid C; use `||`.

**Predict the output #2** – keys 50,30,70,20,40,60,80,65, then delete 20, 30, 50. Output: `60 40 70 65 80`
1. delete 20 – **case 1 (leaf)**: node simply removed.
2. delete 30 – **case 2 (one child)**: its only child 40 takes its place.
3. delete 50 – **case 3 (two children)**: inorder successor = smallest key of right subtree = 60; copy 60 into the root, then delete 60 from the right subtree (60 has one child, 65, so that is case 2).
Final tree: root 60, left 40, right 70 (children 65 and 80).

**Why inorder gives sorted order.** BST rule: left subtree < node < right subtree. Inorder = (left, node, right). By induction: inorder of each subtree is sorted, every key of the left part is smaller than the node and every key of the right part is larger, so the concatenation is sorted.

**Deletion cases and why the inorder successor.** Leaf → remove. One child → splice the child up. Two children → we cannot splice (two subtrees, one slot), so we overwrite the node with a key that keeps the BST rule: the inorder successor is larger than everything in the left subtree and ≤ everything else in the right subtree. It is the leftmost node of the right subtree, so it has **no left child** and can be removed using the easy cases 1/2. (The inorder predecessor works symmetrically.)

**Height.** A binary tree of height h has at most 2^(h+1) − 1 nodes, so n ≤ 2^(h+1) − 1 ⇒ h ≥ ⌈log₂(n+1)⌉ − 1 = ⌊log₂ n⌋ (best case, complete tree). Worst case: every node has one child, h = n − 1.

**Sorted input → degenerate tree.** Each new key is larger than all existing keys, so it always goes to the right of the last node: a chain of height n−1. Search becomes linear. (Measured: 2000 sorted keys → height 1999, average 1000.5 comparisons; random order → height ≈ 20, ≈ 12.5 comparisons.)

**Complete the code** – same 8 keys, heights 3 vs 7 because shape depends only on insertion order: 500,300,700,… fills levels (balanced); 10,20,…,80 forms a chain. Comparisons: 350 → path 500,300,400,350 = 4; 80 in the chain = 8.

**Conclusion.** A plain BST is enough when data is in memory, keys arrive in effectively random order (or the tree is built once from shuffled data), and a bad worst case is tolerable. It must be replaced by AVL / red-black when keys can arrive sorted or nearly sorted (like accession numbers), or when a guaranteed O(log n) bound is required.

**Alternative: sorted array + binary search.** Search O(log n) both. Insert: array O(n) (shifting) vs BST O(h). Sorted listing O(n) both. Space: array stores only keys; BST adds two child pointers per node (16 bytes on 64-bit) but needs no contiguous block and no resizing.

---
## Q2 – AVL trees

**Rotations for 10,20,30,40,50,25:** 10, 20 – none. 30 – **RR** (single left rotation at 10). 40 – none. 50 – **RR** (left rotation at 30). 25 – **RL** (right rotation at 40, then left rotation at 20). Final preorder `30 20 10 25 40 50`, height 3 – matches the expected output.

**Height is O(log n).** Let N(h) be the minimum number of nodes in an AVL tree of height h. The root has one subtree of height h−1 and (at least) one of height h−2, so N(h) = 1 + N(h−1) + N(h−2) – a Fibonacci-like recurrence, N(h) ≥ φ^h/√5 with φ = 1.618. Hence n ≥ φ^h/√5 − 1 ⇒ h ≤ 1.44 log₂(n+2). 

**Deletion.** After a normal BST delete, walk back up to the root, updating heights and rebalancing at each ancestor. Insertion needs at most one (single or double) rotation, but deletion may need a rotation at every level, i.e. O(log n) rotations. 

**Measured (100,000 sorted keys):** BST height 100,000 vs AVL height 17; the BST needs seconds to build (O(n²)), the AVL a few hundredths of a second; 99,983 rotations for the sorted input; ≈0.7 rotations per insertion for random input; balance factor verified at every node. For random input the AVL insert is slower (rotation work) but the tree is half as tall (20 vs ≈40).

**Conclusion.** Yes – on sorted input rotations cost almost nothing compared with a 100,000-deep tree; on random input they roughly cost a constant factor on insert but give a hard guarantee. If input order is unknown or adversarial, the guarantee is worth it.

**Alternative (red-black / skip list).** AVL height ≤ 1.44 log₂ n (tighter, faster lookups, more rotations when updating); red-black ≤ 2 log₂ n (taller, but ≤ 2 rotations per insert, ≤ 3 per delete). Extra space per node: AVL one height (or 2 bits of balance), red-black one colour bit, skip list on average 2 forward pointers per node (p = ½) but no rotations and simple concurrency.

---
## Q3 – B-trees

**Hand insertion (t = 2)** – printed by `q3_btree.c`: 10,20,5 fill the root `[5 10 20]`; inserting 6 splits it (median 10 moves up): `[10] / [5 6] [20]`; 12, 30, 7 go into leaves; inserting 17 splits `[12 20 30]` → median 20 moves up: `[10 20] / [5 6 7] [12 17] [30]`. Two splits in total, height 2.

**Wide and short.** Each node holds many keys (up to m−1) and has up to m children, so height ≈ log_m n; with m = 100 it is about a quarter of log₂ n.

**Splitting and equal leaf depth.** A full node is split into two around the median, which moves up. Only a split of the **root** increases the height, and it adds one level above *all* leaves at once – so every leaf is always at the same depth.

**B vs B+ for range queries.** B+ tree keeps all records in the leaves and links the leaves in a list: find the first key of the week, then follow the leaf links sequentially. A B-tree has keys in internal nodes too, so a range scan must go up and down through the levels repeatedly.

**Node size vs disk block.** A read always fetches a whole block (e.g. 4 KB), so choose the maximum keys per node so that m·(key + child pointer) ≈ block size; using the whole block costs the same as using a part.

**Height for order 100, n = 5,000,000.** Min children = ⌈100/2⌉ = 50 ⇒ h ≤ log₅₀((n+1)/2) = log₅₀(2,500,000) ≈ 3.77 ⇒ at most **4 levels** = 4 disk reads = 40 ms. Best case log₁₀₀(5·10⁶) ≈ 3.35 ⇒ also 4 levels. Balanced BST/AVL: ≈ 23 reads = 230 ms (AVL worst 1.44·log₂ n ≈ 33 → 330 ms).

**Conclusion.** For disk data the cost is the number of block reads, so a B-tree’s fan-out (4 reads) wins. For data in memory a pointer dereference costs nanoseconds, so the AVL tree’s simplicity, small nodes (no wasted slots) and tight height are better.

**Alternative (AVL, one node per block).** 23 vs 4 reads. Each block would hold one key and two pointers (≈ 20 bytes of 4,096 → > 99 % wasted); B-tree nodes are always at least half full.

---
## Q4 – Red-black trees

**Properties:** (1) every node red or black; (2) root black; (3) every NIL leaf black; (4) a red node has only black children; (5) every path from a node to its NIL descendants has the same number of black nodes.

**Insert 10,20,30,15,25,5:** 10 → black root. 20 → red child. 30 → red-red with black (NIL) uncle: **left rotation at 10**, recolour: 20 black, 10 and 30 red. 15 → parent 10 red and uncle 30 red: **recolour** (10, 30 black, 20 red) and the root is turned black again. 25 → parent 30 black: nothing. 5 → parent 10 black: nothing. Final: 20B(10B(5R,15R), 30B(25R)). The program prints every step.

**Height ≤ 2 log₂(n+1).** Let bh be the black-height. Since no red node has a red child, at least half the nodes on any root-leaf path are black: h ≤ 2·bh. A subtree of black-height bh has at least 2^bh − 1 nodes, so n ≥ 2^(h/2) − 1 ⇒ h ≤ 2 log₂(n+1).

**Rotations.** RB: ≤ 2 per insert and ≤ 3 per delete. AVL: ≤ 2 per insert, but up to O(log n) per delete. Measured with 10,000 random operations: RB 0.58 rotations/insert vs AVL 0.69; deletes about equal (0.39 vs 0.38). Both trees had height 16. For ascending input AVL is shorter (14 vs 24).

**Why red-black for frequent updates?** Looser balance ⇒ less restructuring per update and a *constant* bound on rotations, at the price of slightly taller trees.

**Caching the leftmost node.** Keep a `leftmost` pointer in the tree head: on insert, update it if the new node became the new leftmost; on delete of the leftmost, set it to its successor. “Pick next process” becomes O(1) (Linux `rb_root_cached`).

**Engineering decision.** Red-black: bounded work per insert/delete (latency matters in a scheduler), and the measurements show it never loses badly on updates; AVL’s only advantage is a slightly shorter tree for look-ups, which a scheduler hardly uses.

**Alternative (binary min-heap).** Find-min O(1) both; insert O(log n) both; removing an *arbitrary* process (e.g. it exits or blocks): heap O(n) to find it (O(log n) with an index array) vs red-black O(log n); space: heap array only (no pointers) vs 3 pointers + colour per node.

---
## Q5 – Splay trees

**Steps.** *Zig*: x is a child of the root → one rotation. *Zig-zig*: x and its parent are both left (or both right) children → rotate the **parent** first, then x. *Zig-zag*: x is a left child of a right child (or vice versa) → rotate x twice. `q5_splay.c` prints the steps for “access 3 then 9”.

**Amortised O(log n), single op O(n).** With potential Φ = Σ log₂(size of subtree), every splay costs ≤ 3 log₂ n + 1 amortised. A single access can still be O(n): after sorted insertions the tree is a chain and the first access walks it all (measured: 1000 nodes), but the same splay flattens the chain so later accesses are cheap (total for 1000 sequential accesses ≈ 6 per access).

**No balance information.** Restructuring is determined only by the accessed path; nothing is stored in the nodes (AVL: a height, RB: a colour).

**Measured (1,000 keys, 10,000 accesses; avg nodes visited):** 80 % → 5 % of keys: splay 8.05 vs AVL 7.03; uniform: splay 12.4 vs AVL 9.2; 95 % → 1 % of keys: splay 4.1 vs AVL 6.4. So splay wins only when the skew is strong; at 80/5 it is roughly a tie.

**When splay performs badly:** uniform random access (every access pays rotations and nothing is gained); real-time systems (a single O(n) access); concurrent readers (even a read modifies the tree).

**Conclusion.** An adaptive structure beats an always-balanced one when accesses show locality or skew that persists long enough for the restructuring to pay off (caches, recently-used items).

**Alternative (hash table + doubly linked list = LRU).** O(1) average per access vs amortised O(log n); space per item: hash slot + 2 list pointers vs 2 child pointers; only the splay tree can also list items in sorted order.

---
## Q6 – Tries

**Time depends on word length.** Search follows one pointer per character: O(m), whatever n is. (Measured: ≈ 8 ns per 3-letter prefix count for 500, 5,000 and 50,000 names.)

**Memory.** A node = 26 pointers × 8 B + two ints = **216 bytes**. Six contacts → 21 nodes + root = 22 nodes × 216 = **4,752 bytes** for 25 characters of text. Large because most of the 26 pointers are NULL. For 5,000 names: ≈ 2.5 MB vs ≈ 75 KB for a sorted array.

**Comparison.** Trie O(m) search; sorted array + binary search O(m log n) and O(n) insert; BST of strings O(m log n) on average, no guarantee unless balanced.

**Compressed trie (radix tree).** Chains of single-child nodes are merged into one edge labelled with a string, so there are at most 2n−1 nodes regardless of word length.

**Expected output** `a:5 an:4 anu:2 b:1 z:0` is reproduced by `q6_complete.c`.

**Conclusion (phone).** Not the plain 26-pointer trie: it is 30× bigger than the data. Use a compressed or sibling-list trie, or – for only a few thousand contacts – a sorted array (≈ 75 KB) with two binary searches, which is fast enough and tiny.

**Alternative (sorted array + two binary searches).** Prefix query O(m log n) (lower and upper bound) vs O(m) for a trie; adding a contact O(n) vs O(m); memory smaller.

---
## Q7 – Dictionary (200,000 words, phone)

**Stage 1 – Interpret.**
| Operation | Frequency |
|---|---|
| exact look-up | very high (every word typed) |
| autocomplete | very high (every keystroke) |
| spell-check suggestions | medium (only for unknown words) |
| range listing between two words | low (browsing) |
| update (add/remove words) | once a week |

Constraints: phone memory (assume ≤ 20 MB for the dictionary), response < 16 ms per keystroke, updates rare (rebuild is acceptable). Measurable requirements: look-up < 1 ms; first 10 completions < 5 ms; range listing starts < 10 ms; memory ≤ 20 MB for 200,000 words; weekly update < 10 s.

**Stage 2 – Compare** (n words, m word length, k results):
| Structure | Look-up | Autocomplete | Range [a..b] | Insert | Space |
|---|---|---|---|---|---|
| BST | O(m·log n) avg, O(m·n) worst | O(m log n + k) | O(m log n + k) | O(m log n) avg | n nodes × 2 ptrs + strings |
| AVL | O(m log n) | O(m log n + k) | O(m log n + k) | O(m log n) | + height per node |
| Red-black | O(m log n) | O(m log n + k) | O(m log n + k) | O(m log n) | + colour bit |
| B-tree | O(m log n) comparisons, few block reads | O(m log n + k) | O(m log n + k) | O(m log n) | nodes ≥ ½ full; ideal for disk |
| Trie | **O(m)** | **O(m + k·len)** | O(m + output + pruned) | O(m) | O(total characters); 26-ptr nodes are huge |

**Chosen design:** a **trie with sorted first-child/next-sibling lists** (24-byte nodes instead of 216). Look-up is O(m) independent of the dictionary size, autocomplete is a subtree walk, children are sorted so the walk is already alphabetical, range listing prunes whole subtrees, and spell-check generates the ~50·m edit-distance-1 candidates and looks each up (O(m) each).

**Stage 3 – Measured** (`outputs/q7_dictionary.txt`): trie results equal a sorted-array reference on a 1,000-word sample (look-up, autocomplete counts, range counts). At 200,000 words: ≈ 11 MB (sibling trie) vs ≈ 100 MB (26-pointer trie) vs ≈ 3.7 MB (sorted array); look-up ≈ 1 µs; autocomplete of 10 words ≈ 0.5 µs.

**Stage 4 – Defend.** All requirements are met by a wide margin (look-up ~1 µs, 11 MB). For 2 million words the trie would be ≈ 110 MB: switch to a **compressed (radix) trie** or a DAWG / compact array-based trie that also shares suffixes, or keep a front-coded sorted array on flash with a small trie in RAM for the top levels. Remaining limitation: the code handles only letters a–z; a regional script needs UTF-8 (byte- or code-point-based children) and more than edit-distance-1 spell-check needs a bounded edit-distance walk of the trie.

**Alternative (hash table + sorted array).** Look-up O(m) expected (hash); autocomplete/range O(m log n + k) with the sorted array; space: array of pointers (8n) + hash table (≈ 16n) + shared strings ≈ 8–10 MB for 200,000 words – similar. Updates: the weekly update must change both structures (hash insert O(m) per word, sorted array rebuild/merge O(n)), and they must be kept consistent; the trie needs only one structure. 
