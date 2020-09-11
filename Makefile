sources = $(wildcard src/*.c)

all:
	gcc $(sources) -o scc -lm -g
