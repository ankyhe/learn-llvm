#! /usr/bin/env bash

set -e

# `/opt/homebrew/opt/llvm/bin/llvm-config --cxxflags --ldflags --system-libs --libs core` remove -fno-exceptions due to EvaParser.h uses throw
clang++ -o eva-llvm \
  -I/opt/homebrew/Cellar/llvm/20.1.2/include -std=c++17 -stdlib=libc++ -funwind-tables -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS \
  -L/opt/homebrew/Cellar/llvm/20.1.2/lib -Wl,-search_paths_first -Wl,-headerpad_max_install_names \
  -lLLVM-20 -Wno-c++20-extensions \
  eva-llvm.cpp

./eva-llvm

/opt/homebrew/opt/llvm/bin/lli ./out.ll
echo $?
