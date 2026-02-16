#include "druk/codegen/chunk.hpp"

namespace druk {

void Chunk::write(uint8_t byte, int line) {
  code_.push_back(byte);
  lines_.push_back(line);
}

void Chunk::write(OpCode opcode, int line) {
  write(static_cast<uint8_t>(opcode), line);
}

int Chunk::add_constant(Value value) {
  // Simple linear scan for now. For larger pools, use a map.
  for (size_t i = 0; i < constants_.size(); ++i) {
    if (constants_[i] == value) {
      return static_cast<int>(i);
    }
  }
  constants_.push_back(value);
  return static_cast<int>(constants_.size() - 1);
}

void Chunk::patch(size_t offset, uint8_t byte) {
  if (offset < code_.size()) {
    code_[offset] = byte;
  }
}

} // namespace druk
