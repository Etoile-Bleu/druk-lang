#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept> // For std::bad_alloc if needed, though we might prefer returning null or crashing
#include <vector>


namespace druk {

class ArenaAllocator {
public:
  // 4KB default block size
  static constexpr size_t DEFAULT_BLOCK_SIZE = 4096;

  explicit ArenaAllocator(size_t block_size = DEFAULT_BLOCK_SIZE)
      : block_size_(block_size) {
    // Pre-allocate the first block
    allocate_block();
  }

  // Non-copyable
  ArenaAllocator(const ArenaAllocator &) = delete;
  ArenaAllocator &operator=(const ArenaAllocator &) = delete;

  // Movable
  ArenaAllocator(ArenaAllocator &&) noexcept = default;
  ArenaAllocator &operator=(ArenaAllocator &&) noexcept = default;

  ~ArenaAllocator() { reset(); }

  template <typename T, typename... Args>
  [[nodiscard]] T *make(Args &&...args) {
    void *mem = allocate(sizeof(T), alignof(T));
    return new (mem) T(std::forward<Args>(args)...);
  }

  template <typename T> [[nodiscard]] T *allocate_array(size_t count) {
    if (count == 0)
      return nullptr;
    return static_cast<T *>(allocate(count * sizeof(T), alignof(T)));
  }

  template <typename T> [[nodiscard]] T *make_array(size_t count) {
    if (count == 0)
      return nullptr;
    T *arr = allocate_array<T>(count);
    // Default construct each element
    for (size_t i = 0; i < count; i++) {
      new (&arr[i]) T();
    }
    return arr;
  }

  [[nodiscard]] void *allocate(size_t size, size_t alignment) {
    size_t current_ptr =
        reinterpret_cast<size_t>(blocks_.back().get()) + current_offset_;
    size_t padding = (alignment - (current_ptr % alignment)) % alignment;

    if (current_offset_ + padding + size > block_size_) {
      // Allocate new block
      allocate_block();
      current_offset_ = 0;
      // Recalculate padding for new block (always 0 if block is aligned, which
      // malloc usually is)
      current_ptr = reinterpret_cast<size_t>(blocks_.back().get());
      padding = (alignment - (current_ptr % alignment)) % alignment;
    }

    current_offset_ += padding;
    void *result = blocks_.back().get() + current_offset_;
    current_offset_ += size;
    return result;
  }

  void reset() {
    blocks_.clear();
    current_offset_ = 0;
    allocate_block();
  }

private:
  void allocate_block() {
    // Using unique_ptr to manage memory. C++ new[] aligns to max_align_t
    // usually.
    blocks_.emplace_back(std::make_unique<uint8_t[]>(block_size_));
    current_offset_ = 0;
  }

  size_t block_size_;
  std::vector<std::unique_ptr<uint8_t[]>> blocks_;
  size_t current_offset_ = 0;
};

} // namespace druk
