#pragma once

#include <string>
#include <vector>

#include "druk/ir/ir_basic_block.h"
#include "druk/ir/ir_type.h"
#include "druk/ir/ir_value.h"

namespace druk::ir
{

class Module;

/**
 * @brief Represents a function in the IR, containing multiple basic blocks.
 */
class Function : public Value
{
   public:
    Function(const std::string& name, std::shared_ptr<Type> type, Module* parent = nullptr);

    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override
    {
        return type_;
    }

    void addBasicBlock(std::unique_ptr<BasicBlock> block);
    void addParameter(std::unique_ptr<Parameter> param);

    const std::vector<std::unique_ptr<BasicBlock>>& getBasicBlocks() const
    {
        return blocks_;
    }

    const std::vector<std::unique_ptr<Parameter>>& getParameters() const
    {
        return parameters_;
    }

    Parameter* getParameter(size_t index) const
    {
        if (index < parameters_.size())
        {
            return parameters_[index].get();
        }
        return nullptr;
    }

    size_t getParameterCount() const
    {
        return parameters_.size();
    }

    Module* getParent() const
    {
        return parent_;
    }

   private:
    std::shared_ptr<Type>                    type_;
    std::vector<std::unique_ptr<BasicBlock>> blocks_;
    std::vector<std::unique_ptr<Parameter>>  parameters_;
    Module*                                  parent_;
};

}  // namespace druk::ir
