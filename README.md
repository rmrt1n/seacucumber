# Seacucumber

Seacucumber a small interpreter and transpiler I made to learn about how
programming languages work. It is inspired by scheme and ocaml. Several example
Seacucumber code is located in the examples directory.

### Build Instructions
Instructions to build Seacucumber from source.
To actually use the transpiler, you will need OCaml installed on your system.
```bash
# clone this repository
git clone https://github.com/rmrt1n/seacucumber.git
# cd into the repository
cd seacucumber
# build seacucumber
make
```

### Usage
`scc` is the executable for the tree walking interpreter, while `tscc` is the
Seacucumber to OCaml transpiler.
```bash
# run an interactive prompt
./scc
# interpret seacucumber code 
./scc FILENAME
# tscc will compile seacucumber to ocaml,
# and run ocamlc to create an executable
./tscc FILENAME
```
