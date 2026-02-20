#include "druk/ir/ir_instruction_arrays.h"

namespace druk::ir
{

BuildArrayInst::BuildArrayInst(const std::vector<Value*>& elements)
    : Instruction(Opcode::BuildArray)
{
    for (auto* element : elements)
        addOperand(element);
}

std::string BuildArrayInst::toString() const
{
    return "build_array";
}

std::shared_ptr<Type> BuildArrayInst::getType() const
{
    return Type::getVoidTy();
}

IndexGetInst::IndexGetInst(Value* array_val, Value* index_val) : Instruction(Opcode::IndexGet)
{
    addOperand(array_val);
    addOperand(index_val);
}

std::string IndexGetInst::toString() const
{
    return "index_get";
}

std::shared_ptr<Type> IndexGetInst::getType() const
{
    return Type::getVoidTy();
}

IndexSetInst::IndexSetInst(Value* array_val, Value* index_val, Value* value)
    : Instruction(Opcode::IndexSet)
{
    addOperand(array_val);
    addOperand(index_val);
    addOperand(value);
}

std::string IndexSetInst::toString() const
{
    return "index_set";
}

std::shared_ptr<Type> IndexSetInst::getType() const
{
    return Type::getVoidTy();
}

LenInst::LenInst(Value* value) : Instruction(Opcode::Len)
{
    addOperand(value);
}

std::string LenInst::toString() const
{
    return "len";
}

std::shared_ptr<Type> LenInst::getType() const
{
    return Type::getVoidTy();
}

}  // namespace druk::ir
