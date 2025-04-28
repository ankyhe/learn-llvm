#include "./src/EvaLLVM.h"

int main(int argc, char* argv[]) {
	std::string program = R"(
    // (print "VERSION: %d TRUE: %d FALSE: %d\n" VERSION TRUE FALSE)
    (print "CONSTANT1: %s\n" (var CONSTANT1 "2024-04-28"))
  )";

	EvaLLVM evaLLVM;
	evaLLVM.exec(program);

	return 0;
}
