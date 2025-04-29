#include "./src/EvaLLVM.h"

int main(int argc, char* argv[]) {
	std::string program = R"(

    (var VERSION 42)

    (begin
       (var (VERSION string) "2025-04-29")
       (print "VERSION: %s\n" VERSION)
    )
   
    (print "VERSION: %d\n" VERSION)

  )";

	EvaLLVM evaLLVM;
	evaLLVM.exec(program);

	return 0;
}
