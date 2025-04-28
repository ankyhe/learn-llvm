#! /usr/bin/env bash

./node_modules/syntax-cli/bin/syntax -g src/parser/EvaGrammar.bnf -m LALR1 -o src/parser/EvaParser.h
