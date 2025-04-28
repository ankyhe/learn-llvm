#! /usr/bin/env bash

make all

./eva-llvm

/opt/homebrew/opt/llvm/bin/lli ./out.ll
echo $?
