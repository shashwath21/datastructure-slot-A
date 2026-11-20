BIN     = ./bin
CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -g
SAN     = -fsanitize=address,undefined -fno-omit-frame-pointer
PROGS   = q1_predict1 q1_predict2 q1_complete q1_bst \
          q2_complete q2_avl q3_btree q4_rbtree q5_splay \
          q6_complete q6_trie q7_dictionary

all: $(PROGS:%=$(BIN)/%)

$(BIN)/%: src/%.c
	@mkdir -p $(BIN)
	$(CC) $(CFLAGS) $(SAN) -o $@ $< -lm

# Run everything and save the output of each program in outputs/
run: all
	@mkdir -p outputs data
	@for p in $(PROGS); do \
	  echo ">>> running $$p"; \
	  $(BIN)/$$p > outputs/$$p.txt 2>&1 || { echo "FAILED: $$p"; cat outputs/$$p.txt; exit 1; }; \
	done
	@echo "All programs ran OK. Outputs are in outputs/"

clean:
	rm -rf $(BIN)
