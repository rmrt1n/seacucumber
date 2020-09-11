tree-walk = $(filter-out src/transpiler.c, $(wildcard src/*.c))
transpiler = $(filter-out src/main.c, $(wildcard src/*.c))

all: scc tscc

scc:
	gcc $(tree-walk) -o scc -lm -g

tscc:
	gcc $(transpiler) -o tscc -lm -g
