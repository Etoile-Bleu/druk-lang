#include "druk/ir/ir_function.h"

#include "druk/ir/ir_basic_block.h"
#include "druk/ir/ir_type.h"


namespace druk::ir
{

Function::Function(const std::string& name, std::shared_ptr<Type> type, Module* parent)
    : type_(type), parent_(parent)
{
    setName(name);
}

std::string Function::toString() const
{
    std::string result = "define " + type_->toString() + " @" + getName() + " (";

    // Add parameters
    for (size_t i = 0; i < parameters_.size(); ++i)
    {
        if (i > 0)
            result += ", ";
        result += parameters_[i]->getType()->toString() + " " + parameters_[i]->toString();
    }

    result += ")";
    return result;
}

void Function::addBasicBlock(std::unique_ptr<BasicBlock> block)
{
    block->setParent(this);
    blocks_.push_back(std::move(block));
}

void Function::addParameter(std::unique_ptr<Parameter> param)
{
    parameters_.push_back(std::move(param));
}

}  // namespace druk::ir
