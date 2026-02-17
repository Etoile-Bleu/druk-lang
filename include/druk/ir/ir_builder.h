#pragma once

#include <memory>

#include "druk/ir/ir_basic_block.h"
#include "druk/ir/ir_instruction.h"

namespace druk::ir
{

/**
 * @brief Helper class to build IR instructions into basic blocks.
 */
class IRBuilder
{
   public:
    IRBuilder() : insert_block_(nullptr) {}

    void setInsertPoint(BasicBlock* block)
    {
        insert_block_ = block;
    }
    BasicBlock* getInsertBlock() const
    {
        return insert_block_;
    }

    // Instruction creation methods
    Instruction* createAdd(Value* left, Value* right, const std::string& name = "");
    Instruction* createSub(Value* left, Value* right, const std::string& name = "");
    Instruction* createMul(Value* left, Value* right, const std::string& name = "");
    Instruction* createDiv(Value* left, Value* right, const std::string& name = "");
    
    // Comparison instructions
    Instruction* createEqual(Value* left, Value* right, const std::string& name = "");
    Instruction* createNotEqual(Value* left, Value* right, const std::string& name = "");
    Instruction* createLessThan(Value* left, Value* right, const std::string& name = "");
    Instruction* createLessEqual(Value* left, Value* right, const std::string& name = "");
    Instruction* createGreaterThan(Value* left, Value* right, const std::string& name = "");
    Instruction* createGreaterEqual(Value* left, Value* right, const std::string& name = "");

    Instruction* createAlloca(std::shared_ptr<Type> type, const std::string& name = "");
    Instruction* createLoad(Value* ptr, const std::string& name = "");
    Instruction* createStore(Value* val, Value* ptr);

    Instruction* createBranch(BasicBlock* dest);
    Instruction* createCondBranch(Value* cond, BasicBlock* trueDest, BasicBlock* falseDest);
    Instruction* createRet(Value* val = nullptr);

    Instruction* createCall(Function* func, const std::vector<Value*>& args,
                            const std::string& name = "");

    Instruction* createPrint(Value* val);

   private:
    BasicBlock* insert_block_;

    void insert(std::unique_ptr<Instruction> inst);
};

}  // namespace druk::ir
