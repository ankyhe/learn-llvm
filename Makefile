all:
	clang++ -o eva-llvm `/opt/homebrew/opt/llvm/bin/llvm-config --cxxflags --ldflags --system-libs --libs core` eva-llvm.cpp
