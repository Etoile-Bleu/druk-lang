#pragma once

#include "druk/ir/ir_instruction_base.h"

namespace druk::ir
{

class BinaryInst : public Instruction
{
   public:
    BinaryInst(Opcode op, Value* l, Value* r);
    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override;
};

}  // namespace druk::ir
