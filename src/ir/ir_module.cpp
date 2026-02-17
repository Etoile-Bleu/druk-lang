#include "druk/ir/ir_module.h"

namespace druk::ir
{

Module::Module(const std::string& name) : name_(name) {}

void Module::addFunction(std::unique_ptr<Function> func)
{
    functions_[func->getName()] = std::move(func);
}

Function* Module::getFunction(const std::string& name) const
{
    auto it = functions_.find(name);
    return it != functions_.end() ? it->second.get() : nullptr;
}

}  // namespace druk::ir
