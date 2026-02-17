#include "druk/ir/ir_instruction.h"

#include "druk/ir/ir_type.h"

namespace druk::ir
{

BinaryInst::BinaryInst(Opcode op, Value* l, Value* r) : Instruction(op)
{
    addOperand(l);
    addOperand(r);
}

std::string BinaryInst::toString() const
{
    return "binop";
}

std::shared_ptr<Type> BinaryInst::getType() const
{
    if (getOperands().empty())
        return Type::getVoidTy();
    return getOperands()[0]->getType();
}

AllocaInst::AllocaInst(std::shared_ptr<Type> type)
    : Instruction(Opcode::Alloca), allocatedType_(type)
{
}

std::string AllocaInst::toString() const
{
    return "alloca";
}

std::shared_ptr<Type> AllocaInst::getType() const
{
    return allocatedType_;
}

LoadInst::LoadInst(Value* ptr) : Instruction(Opcode::Load)
{
    addOperand(ptr);
}

std::string LoadInst::toString() const
{
    return "load";
}

std::shared_ptr<Type> LoadInst::getType() const
{
    return Type::getVoidTy();  // Should be pointee type, simplified for now
}

StoreInst::StoreInst(Value* val, Value* ptr) : Instruction(Opcode::Store)
{
    addOperand(val);
    addOperand(ptr);
}

std::string StoreInst::toString() const
{
    return "store";
}

std::shared_ptr<Type> StoreInst::getType() const
{
    return Type::getVoidTy();
}

BranchInst::BranchInst(BasicBlock* dest) : Instruction(Opcode::Branch), dest_(dest) {}

std::string BranchInst::toString() const
{
    return "br";
}

std::shared_ptr<Type> BranchInst::getType() const
{
    return Type::getVoidTy();
}

CondBranchInst::CondBranchInst(Value* cond, BasicBlock* t, BasicBlock* f)
    : Instruction(Opcode::ConditionalBranch), trueDest_(t), falseDest_(f)
{
    addOperand(cond);
}

std::string CondBranchInst::toString() const
{
    return "condbr";
}

std::shared_ptr<Type> CondBranchInst::getType() const
{
    return Type::getVoidTy();
}

RetInst::RetInst(Value* val) : Instruction(Opcode::Return)
{
    if (val)
        addOperand(val);
}

std::string RetInst::toString() const
{
    return "ret";
}

std::shared_ptr<Type> RetInst::getType() const
{
    return Type::getVoidTy();
}

CallInst::CallInst(Function* func, const std::vector<Value*>& args)
    : Instruction(Opcode::Call), func_(func)
{
    for (auto* arg : args) addOperand(arg);
}

std::string CallInst::toString() const
{
    return "call";
}

std::shared_ptr<Type> CallInst::getType() const
{
    return Type::getVoidTy();  // Should be return type
}

PrintInst::PrintInst(Value* val) : Instruction(Opcode::Print)
{
    addOperand(val);
}

std::string PrintInst::toString() const
{
    return "print";
}

std::shared_ptr<Type> PrintInst::getType() const
{
    return Type::getVoidTy();
}

}  // namespace druk::ir
