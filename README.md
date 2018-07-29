# QLex
Lexcial Analyzer Generator written in C++. It can generate a simple C99 lexer.
## Preprocessor.hpp
Basic file I/O processor, for processing lex.l and y.tab.h.

Regular expressions with `[]^-?.*+|()${}%<>` are converted to the regular expressions with only `()*|$`, where label `$` is used as the ending symbol of a regular expression.

This part is written by Jiahuan Song.
## To be continue...
