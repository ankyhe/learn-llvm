#ifndef EVALLVM_H
#define EVALLVM_H

#include <iostream>
#include <string>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Verifier.h>

class EvaLLVM {

	public:
		EvaLLVM() {
			initModule();

			setupExternalFunctions();
		}

		void exec(const std::string& program) {
			compile();

			module->print(llvm::outs(), nullptr);

			std::cout << std::endl;

		  saveModuleToFile("./out.ll");

		}

		void compile(/* ast */) {
			fn = createFunction("main", llvm::FunctionType::get(builder->getInt32Ty(), false /* vararg */ ));

			auto result = gen();
			
			builder->CreateRet(builder->getInt32(0));
		}

		llvm::Value* gen() {
			auto str = builder->CreateGlobalString("Hello World\n");

			auto printfFn = module->getFunction("printf");
			std::vector<llvm::Value *> args{str};

			return builder->CreateCall(printfFn, args);
		}

		llvm::Function* createFunction(const std::string &fnName, llvm::FunctionType *fnType) {
			auto fn = module->getFunction(fnName);
			
			if (fn == nullptr) {
				fn = createFunctionProto(fnName, fnType);
			}
			createFunctionBlock(fn);

			return fn;
		}

		llvm::Function* createFunctionProto(const std::string &fnName, llvm::FunctionType *fnType) {
			auto fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, fnName, *module);

			verifyFunction(*fn);

			return fn;
		}

	  void createFunctionBlock(llvm::Function *fn) {
			auto entry = createBasicBlock("entry", fn);
			builder->SetInsertPoint(entry);
		}

    llvm::BasicBlock* createBasicBlock(const std::string &name, llvm::Function *fn = nullptr) {
			return llvm::BasicBlock::Create(*ctx, name, fn);
		}

		void setupExternalFunctions() {
		  auto bytePtrTy = llvm::PointerType::get(builder->getInt8Ty(), 0);

			// int printf(const char* fmt, ...)
			module->getOrInsertFunction("printf", 
					                        llvm::FunctionType::get(builder->getInt32Ty(), 
																		 bytePtrTy,
																		 true));
		}

	private:

		void saveModuleToFile(const std::string& filename) {
			std::error_code errorCode;
			llvm::raw_fd_ostream outLL(filename, errorCode);
			module->print(outLL, nullptr);
		}

		void initModule() {
			ctx = std::make_unique<llvm::LLVMContext>();
			module = std::make_unique<llvm::Module>("EvaLLVM", *ctx);
			builder = std::make_unique<llvm::IRBuilder<>>(*ctx);
		}

		llvm::Function *fn;

		std::unique_ptr<llvm::LLVMContext> ctx;
		std::unique_ptr<llvm::Module> module;
		std::unique_ptr<llvm::IRBuilder<>> builder;
};

#endif //EVALLVM_H
