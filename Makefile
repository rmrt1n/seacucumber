tree-walk = $(filter-out src/transpiler.c, $(wildcard src/*.c))
transpiler = $(filter-out src/main.c, $(wildcard src/*.c))

scc:
	gcc $(tree-walk) -o bin/scc -lm -g

tscc:
	gcc $(transpiler) -o bin/tscc -lm -g
