# Seacucumber

Seacucumber a small interpreter and transpiler I made to learn about how
programming languages work. It is inspired by scheme and ocaml. Several example
Seacucumber code is located in the examples directory.

![Screeenshot of the language & interpreter](/screenshot.jpg)

**Built with:**

![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)


## Getting Started
Instructions to build Seacucumber from source. To actually use the transpiler, 
you will need to have [OCaml](https://ocaml.org/) installed on your system.
```bash
# clone this repository
git clone https://github.com/rmrt1n/seacucumber.git

# cd into the repository
cd seacucumber

# build seacucumber
make
```

## Usage
`scc` is the executable for the tree walking interpreter, while `tscc` is the 
Seacucumber-to-OCaml transpiler. Below are the commmands to run the programs.
For examples of programs written in Seacucumber, checkout the
[examples](/examples) directory.
```bash
# run an interactive prompt
./scc

# interpret seacucumber code 
./scc FILENAME

# tscc will compile seacucumber to ocaml,
# and run ocamlc to create an executable
./tscc FILENAME
```

## Language Grammar
Here is the BNF grammar of Seacucumber:
```
program -> form* EOF

form -> (expression | assignment)

assignment -> "let" IDENT "=" expression

expression -> "if" logic_or "then" expression (else expression)?
            | "fn" "(" params? ")" "->" expression
            | block
            | logic_or

params -> IDENT ("," IDENT)*

block -> "do" form* "done" ";"

logic_or -> logic_and ("or" logic_and)*

logic_and -> equality ("and" equality)*

equality -> comparison (("==" | "!=") comparison)*

comparison -> addition (("<" | ">" | "<=" | ">=") addition)*

addition -> multiplication (("+" | "-") multiplication)*

multiplication -> unary (("*" | "/") unary)*

unary -> ("!" | "-") unary | call

call -> primary ("(" args? ")")*

args -> expression ("," expression)*

primary -> NUMBER | STRING | IDENT
         | "true" | "false" | "nil"
         | "(" expression ")"
```

## License
Distributed under the MIT License. See [LICENSE](/LICENSE) for more information.
