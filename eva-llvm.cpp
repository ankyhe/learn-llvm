#include "./src/EvaLLVM.h"

int main(int argc, char* argv[]) {
	std::string program = R"(
    (print "Value: %d\n" 42)
  )";

	EvaLLVM evaLLVM;
	evaLLVM.exec(program);

	return 0;
}
