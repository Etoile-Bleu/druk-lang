#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "druk/codegen/core/opcode.h"
#include "druk/codegen/core/value.h"

namespace druk::codegen
{

class Chunk
{
   public:
    void write(uint8_t byte, int line);
    void write(OpCode opcode, int line);
    int  addConstant(Value value);

    // Patch a byte at an offset (used for jumps)
    void patch(size_t offset, uint8_t byte);

    const std::vector<Value>& constants() const
    {
        return constants_;
    }

    const std::vector<uint8_t>& code() const
    {
        return code_;
    }
    const std::vector<int>& lines() const
    {
        return lines_;
    }

    // Helper to get byte at offset
    uint8_t at(size_t offset) const
    {
        return code_[offset];
    }

    // Storage for string constants (for deserialization)
    std::vector<std::string>& stringStorage()
    {
        return stringStorage_;
    }
    const std::vector<std::string>& stringStorage() const
    {
        return stringStorage_;
    }

   private:
    std::vector<uint8_t>     code_;
    std::vector<int>         lines_;
    std::vector<Value>       constants_;
    std::vector<std::string> stringStorage_;  // Owns string data for deserialization
};

}  // namespace druk::codegen
