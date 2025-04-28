#ifndef EVALLVM_H
#define EVALLVM_H

#include <iostream>
#include <regex>
#include <string>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Verifier.h>

#include "./parser/EvaParser.h"

using syntax::EvaParser;

class EvaLLVM {

	public:
		EvaLLVM(): parser(std::make_unique<EvaParser>()) {
			initModule();

			setupExternalFunctions();

			createGlobalVar("TRUE", builder->getInt32(1));
			createGlobalVar("FALSE", builder->getInt32(0));
			createGlobalVar("VERSION", builder->getInt32(1024));
		}

		void exec(const std::string& program) {
			auto ast = parser->parse(program);
			compile(ast);

			module->print(llvm::outs(), nullptr);

			std::cout << std::endl;

		  saveModuleToFile("./out.ll");

		}

		void compile(const Exp &exp) {
			fn = createFunction("main", llvm::FunctionType::get(builder->getInt32Ty(), false /* vararg */ ));

			auto result = gen(exp);
			
			builder->CreateRet(builder->getInt32(0));
		}

		llvm::Value* gen(const Exp &exp) {
			switch (exp.type) {
				case NUMBER: return builder->getInt32(exp.number);
				case STRING: {
				  auto re = std::regex("\\\\n");
					auto str = std::regex_replace(exp.string, re, "\n");

				  return builder->CreateGlobalString(str);
				}

				case SYMBOL: {
				  return module->getNamedGlobal(exp.string)->getInitializer();
				}
			  case LIST: {
				  if(exp.list.empty()) {
						return builder->getInt32(0);
					}
          
				  auto tag = exp.list[0];
					if (tag.type == SYMBOL) {
						auto op = tag.string;
						if (op == "printf" || op == "print") {
							std::vector<llvm::Value *> args;
							for (auto i = 1; i < exp.list.size(); ++i) {
								args.push_back(gen(exp.list[i]));
							}
			        auto printfFn = module->getFunction("printf");
			        return builder->CreateCall(printfFn, args);
						} else if (op == "var") {
							auto variableName = exp.list[1].string;
							auto init = gen(exp.list[2]);
							return createGlobalVar(variableName, ((llvm::Constant *)init))->getInitializer();
						}

						// TODO
				    throw std::runtime_error("Not Supported");
					} else {
						// TODO
				    throw std::runtime_error("Not Supported");
					}
				}
				default: throw std::runtime_error("IlleaglStatement: never reach here");
			}
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

		llvm::GlobalVariable* createGlobalVar(const std::string &name, llvm::Constant *init) {
			module->getOrInsertGlobal(name, init->getType());
			auto variable = module->getNamedGlobal(name);
			variable->setAlignment(llvm::MaybeAlign(4));
			variable->setConstant(false);
			variable->setInitializer(init);
			return variable;
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

		std::unique_ptr<EvaParser> parser;
};

#endif //EVALLVM_H
