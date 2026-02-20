#include "druk/ir/ir_builder.h"

#include "druk/ir/ir_instruction_arrays.h"

namespace druk::ir
{

Instruction* IRBuilder::createBuildArray(const std::vector<Value*>& elements,
                                        const std::string& name)
{
    auto inst = std::make_unique<BuildArrayInst>(elements);
    inst->setName(name);
    auto ptr = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createIndex(Value* array_val, Value* index_val,
                                   const std::string& name)
{
    auto inst = std::make_unique<IndexGetInst>(array_val, index_val);
    inst->setName(name);
    auto ptr = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createIndexSet(Value* array_val, Value* index_val, Value* value)
{
    auto inst = std::make_unique<IndexSetInst>(array_val, index_val, value);
    auto ptr = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createLen(Value* value, const std::string& name)
{
    auto inst = std::make_unique<LenInst>(value);
    inst->setName(name);
    auto ptr = inst.get();
    insert(std::move(inst));
    return ptr;
}

}  // namespace druk::ir
