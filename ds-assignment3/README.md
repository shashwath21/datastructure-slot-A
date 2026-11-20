# Data Structures – Assignment 3 (CO3): Tree-Based Storage and Retrieval

**Student:** _your name_ &nbsp;|&nbsp; **Reg. No.:** _your number_ &nbsp;|&nbsp; **Dept/Section:** _..._

C implementations for Q1–Q7 (BST, AVL, B-tree, Red-black, Splay, Trie, Dictionary design).
Every program is compiled with **AddressSanitizer + UBSan** and the outputs are saved in [`outputs/`](outputs/).

## Run it

**On GitHub (automatic):** push this repo → open the **Actions** tab → the workflow *“Build and run all programs”* compiles and runs everything and shows each program’s output (also downloadable as the `program-outputs` artifact).

**On your own machine:**
```bash
make run        # builds into ./bin, runs all programs, writes outputs/*.txt   (~20 s)
make clean
# or a single program:
gcc -O2 -fsanitize=address,undefined -o q1 src/q1_bst.c -lm && ./q1
```
Needs only `gcc` and `make` (Linux / macOS / WSL).

## Push to GitHub
```bash
git init
git add .
git commit -m "Data Structures Assignment 3"
git branch -M main
git remote add origin https://github.com/<your-username>/<repo-name>.git
git push -u origin main
```

## What is where

| Q | File(s) in `src/` | What it does |
|---|---|---|
| Q1 | `q1_bst.c` | BST insert/search/height/delete (3 cases), 4 traversals, save-to-file + rebuild, empty/duplicate tests, 2000 keys random vs sorted |
| Q1 | `q1_predict1.c`, `q1_predict2.c` | the two “Predict the Output” programs (the `││` in the Word file must be `\|\|` in C) |
| Q1 | `q1_complete.c` | “Complete the Code” – TODO 1 & 2 filled in |
| Q2 | `q2_complete.c` | rotate_right / rotate_left TODOs |
| Q2 | `q2_avl.c` | AVL step-by-step insertion of 10,20,30,40,50,25; 100,000 sorted keys BST vs AVL; rotations, timings, balance check |
| Q3 | `q3_btree.c` | B-tree (t=2) insert with splits printed after every key, search incl. min/max/missing, height & disk-read maths for 5 M keys |
| Q4 | `q4_rbtree.c` | Red-black insert + delete, property checker after **every** operation, trace of 10,20,30,15,25,5, RB vs AVL rotation counts |
| Q5 | `q5_splay.c` | splay with zig / zig-zig / zig-zag trace, skewed vs uniform workloads vs AVL, sorted access |
| Q6 | `q6_complete.c` | “Complete the Code” – trie TODOs (run under ASan) |
| Q6 | `q6_trie.c` | trie insert/search/prefix count/autocomplete/delete, 5,000 names, timing for 500/5k/50k names, memory vs sorted array |
| Q7 | `q7_dictionary.c` | dictionary: exact look-up, autocomplete, range listing, spell-check; verified on 1,000 words; memory/time up to 200,000 words |

`data/` holds the generated input files (`names_5000.txt`, `dictionary_1000.txt`). `outputs/tree_saved.txt` is the BST saved by Q1.

## Written answers
All the “Design & Analysis”, “Conclusion”, “Predict the Output” explanations and the “Alternative Concept” analyses are in
**[`docs/ANSWERS.md`](docs/ANSWERS.md)**. Use them as notes for your report, but put them in your own words – you may be asked to explain any part in the viva.

## Still to do by you (cannot be generated)
- Fill in your name/register number above and in the Word file.
- Draw the trees by hand or with a tool (Q1 predictions, Q2 rotations, Q3 splits, Q4 recolouring, Q5 splay steps) – the programs print each tree as text to copy from.
- Write your predicted outputs *before* running, then compare with `outputs/q1_predict1.txt` and `outputs/q1_predict2.txt`.
- Take screenshots of the runs (or of the GitHub Actions log) and add them to a `screenshots/` folder.
- Timings differ per machine – quote the numbers from your own run.
