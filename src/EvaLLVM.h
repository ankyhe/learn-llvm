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
#include "./Environment.h"

using syntax::EvaParser;
using Env = std::shared_ptr<Environment>;

class EvaLLVM {

	public:
		EvaLLVM(): parser(std::make_unique<EvaParser>()) {
			initModule();

			setupExternalFunctions();
			setupGlobalEnvironment();
		}

		void exec(const std::string& program) {
			auto ast = parser->parse("(begin " + program + ")");
			compile(ast, this->globalEnv);

			module->print(llvm::outs(), nullptr);

			std::cout << std::endl;

		  saveModuleToFile("./out.ll");

		}

		void compile(const Exp &exp, Env env) {
			fn = createFunction("main", 
					llvm::FunctionType::get(builder->getInt32Ty(), false /* vararg */ ),
					env);

			auto result = gen(exp, env);
			
			builder->CreateRet(builder->getInt32(0));
		}

		llvm::Value* gen(const Exp &exp, Env env) {
			switch (exp.type) {
				case NUMBER: return builder->getInt32(exp.number);
				case STRING: {
				  auto re = std::regex("\\\\n");
					auto str = std::regex_replace(exp.string, re, "\n");

				  return builder->CreateGlobalString(str);
				}

				case SYMBOL: {
					auto varName = exp.string;
					auto value = env->lookup(varName);

					if (auto globalVar = llvm::dyn_cast<llvm::GlobalVariable>(value)) {
						return builder->CreateLoad(globalVar->getInitializer()->getType(),
								  globalVar,
									varName.c_str());
					}

					// TODO local variable
			    throw std::runtime_error("Not Supported");
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
								args.push_back(gen(exp.list[i], env));
							}
			        auto printfFn = module->getFunction("printf");
			        return builder->CreateCall(printfFn, args);
						} else if (op == "var") {
							auto variableName = exp.list[1].string;
							auto init = gen(exp.list[2], env);
							return createGlobalVar(variableName, ((llvm::Constant *)init))->getInitializer();
						} else if (op == "begin") {
							llvm::Value *blockResources;
							for (int i = 1; i < exp.list.size(); ++i) {
								blockResources = gen(exp.list[i], env);
							}
							return blockResources;
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

		llvm::Function* createFunction(const std::string &fnName, 
				llvm::FunctionType *fnType,
				Env env) {
			auto fn = module->getFunction(fnName);
			
			if (fn == nullptr) {
				fn = createFunctionProto(fnName, fnType, env);
			}
			createFunctionBlock(fn);

			return fn;
		}

		llvm::Function* createFunctionProto(const std::string &fnName, 
				llvm::FunctionType *fnType,
				Env env) {
			auto fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, fnName, *module);
			verifyFunction(*fn);

			env->define(fnName, fn);

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

		void setupGlobalEnvironment() {
			std::map<std::string, llvm::Value *> globalObjects {
				{"VERSION", builder->getInt32(1024)},
				{"TRUE", builder->getInt32(1)},
				{"FALSE", builder->getInt32(0)},
			};

			std::map<std::string, llvm::Value *> globalRecords;
			for (auto &entry : globalObjects) {
				globalRecords[entry.first] = createGlobalVar(entry.first, (llvm::Constant *)entry.second);
			}

			globalEnv = std::make_shared<Environment>(globalRecords, nullptr);
		}

		llvm::Function *fn;

		std::unique_ptr<llvm::LLVMContext> ctx;
		std::unique_ptr<llvm::Module> module;
		std::unique_ptr<llvm::IRBuilder<>> builder;

		std::unique_ptr<EvaParser> parser;
		std::shared_ptr<Environment> globalEnv;
};

#endif //EVALLVM_H
