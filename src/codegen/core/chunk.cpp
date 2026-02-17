#include "druk/codegen/core/chunk.h"

namespace druk::codegen
{

void Chunk::write(uint8_t byte, int line)
{
    code_.push_back(byte);
    lines_.push_back(line);
}

void Chunk::write(OpCode opcode, int line)
{
    code_.push_back(static_cast<uint8_t>(opcode));
    lines_.push_back(line);
}

int Chunk::addConstant(Value value)
{
    // Check if constant already exists (simple optimization)
    for (size_t i = 0; i < constants_.size(); i++)
    {
        if (constants_[i] == value)
            return static_cast<int>(i);
    }
    constants_.push_back(value);
    return static_cast<int>(constants_.size() - 1);
}

void Chunk::patch(size_t offset, uint8_t byte)
{
    if (offset < code_.size())
    {
        code_[offset] = byte;
    }
}

}  // namespace druk::codegen
