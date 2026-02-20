#include "druk/ir/ir_instruction_control.h"

namespace druk::ir
{

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
    return Type::getVoidTy();
}

DynamicCallInst::DynamicCallInst(Value* callee, const std::vector<Value*>& args)
    : Instruction(Opcode::DynamicCall)
{
    addOperand(callee);
    for (auto* arg : args) addOperand(arg);
}

std::string DynamicCallInst::toString() const
{
    return "dcall";
}

std::shared_ptr<Type> DynamicCallInst::getType() const
{
    return Type::getVoidTy();
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
