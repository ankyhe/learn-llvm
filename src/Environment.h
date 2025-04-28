#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <map>
#include <memory>

#include <llvm/IR/Value.h>

#include "Logger.h"

class Environment : public std::enable_shared_from_this<Environment> {

	public:
		Environment(std::map<std::string, llvm::Value *>record, 
				        std::shared_ptr<Environment> parent)
			: record_(record), parent_(parent) {}

		llvm::Value * lookup(const std::string &name) {
			auto env = resolve(name);

			if (env == nullptr) {
				return nullptr;
			}

			return env->record_[name];
		}

		llvm::Value * define(const std::string &name, llvm::Value *value) {
			record_[name] = value;
			return value;
		}

	private:

		std::shared_ptr<Environment> resolve(const std::string &name) {
			if (record_.find(name) != record_.end()) {
				return shared_from_this();
			}

			if (parent_ == nullptr) {
				DIE << "Variable \"" << name << "\" is not defined.";
			}

			return parent_->resolve(name);
		}

		std::map<std::string, llvm::Value *> record_;
		std::shared_ptr<Environment> parent_;
};

#endif //ENVIRONMENT_H
