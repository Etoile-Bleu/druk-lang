#include "druk/ir/ir_builder.h"

#include <iostream>

#include "druk/ir/ir_type.h"

namespace druk::ir
{

void IRBuilder::insert(std::unique_ptr<Instruction> inst)
{
    if (insert_block_)
    {
        insert_block_->appendInstruction(std::move(inst));
    }
}

Instruction* IRBuilder::createAdd(Value* left, Value* right, const std::string& name)
{
    auto inst = std::make_unique<BinaryInst>(Opcode::Add, left, right);
    inst->setName(name);
    auto ptr = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createSub(Value* left, Value* right, const std::string& name)
{
    auto inst = std::make_unique<BinaryInst>(Opcode::Sub, left, right);
    inst->setName(name);
    auto ptr = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createMul(Value* left, Value* right, const std::string& name)
{
    auto inst = std::make_unique<BinaryInst>(Opcode::Mul, left, right);
    inst->setName(name);
    auto ptr = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createDiv(Value* left, Value* right, const std::string& name)
{
    auto inst = std::make_unique<BinaryInst>(Opcode::Div, left, right);
    inst->setName(name);
    auto ptr = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createEqual(Value* left, Value* right, const std::string& name)
{
    auto inst = std::make_unique<BinaryInst>(Opcode::Equal, left, right);
    inst->setName(name);
    auto ptr = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createNotEqual(Value* left, Value* right, const std::string& name)
{
    auto inst = std::make_unique<BinaryInst>(Opcode::NotEqual, left, right);
    inst->setName(name);
    auto ptr = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createLessThan(Value* left, Value* right, const std::string& name)
{
    auto inst = std::make_unique<BinaryInst>(Opcode::LessThan, left, right);
    inst->setName(name);
    auto ptr = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createLessEqual(Value* left, Value* right, const std::string& name)
{
    auto inst = std::make_unique<BinaryInst>(Opcode::LessEqual, left, right);
    inst->setName(name);
    auto ptr = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createGreaterThan(Value* left, Value* right, const std::string& name)
{
    auto inst = std::make_unique<BinaryInst>(Opcode::GreaterThan, left, right);
    inst->setName(name);
    auto ptr = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createGreaterEqual(Value* left, Value* right, const std::string& name)
{
    auto inst = std::make_unique<BinaryInst>(Opcode::GreaterEqual, left, right);
    inst->setName(name);
    auto ptr = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createAlloca(std::shared_ptr<Type> type, const std::string& name)
{
    auto inst = std::make_unique<AllocaInst>(type);
    inst->setName(name);
    auto ptr = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createLoad(Value* ptr, const std::string& name)
{
    auto inst = std::make_unique<LoadInst>(ptr);
    inst->setName(name);
    auto ptr_inst = inst.get();  // rename to avoid shadow
    insert(std::move(inst));
    return ptr_inst;
}

Instruction* IRBuilder::createStore(Value* val, Value* ptr)
{
    auto inst     = std::make_unique<StoreInst>(val, ptr);
    auto ptr_inst = inst.get();
    insert(std::move(inst));
    return ptr_inst;
}

Instruction* IRBuilder::createBranch(BasicBlock* dest)
{
    auto inst = std::make_unique<BranchInst>(dest);
    auto ptr  = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createCondBranch(Value* cond, BasicBlock* trueDest, BasicBlock* falseDest)
{
    auto inst = std::make_unique<CondBranchInst>(cond, trueDest, falseDest);
    auto ptr  = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createRet(Value* val)
{
    auto inst = std::make_unique<RetInst>(val);
    auto ptr  = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createCall(Function* func, const std::vector<Value*>& args,
                                   const std::string& name)
{
    auto inst = std::make_unique<CallInst>(func, args);
    inst->setName(name);
    auto ptr = inst.get();
    insert(std::move(inst));
    return ptr;
}

Instruction* IRBuilder::createPrint(Value* val)
{
    auto inst = std::make_unique<PrintInst>(val);
    auto ptr  = inst.get();
    insert(std::move(inst));
    return ptr;
}

}  // namespace druk::ir
