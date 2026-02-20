#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

namespace druk::util {

/**
 * @brief ArenaAllocator provides fast, region-based memory allocation.
 */
class ArenaAllocator {
public:
  static constexpr size_t kDefaultBlockSize = 4096;

  explicit ArenaAllocator(size_t blockSize = kDefaultBlockSize)
      : blockSize_(blockSize) {
    allocateBlock();
  }

  ArenaAllocator(const ArenaAllocator &) = delete;
  ArenaAllocator &operator=(const ArenaAllocator &) = delete;

  ArenaAllocator(ArenaAllocator &&) noexcept = default;
  ArenaAllocator &operator=(ArenaAllocator &&) noexcept = default;

  ~ArenaAllocator() { reset(); }

  /**
   * @brief Constructs an object of type T in the arena.
   */
  template <typename T, typename... Args>
  [[nodiscard]] T *make(Args &&...args) {
    void *mem = allocate(sizeof(T), alignof(T));
    return new (mem) T(std::forward<Args>(args)...);
  }

  /**
   * @brief Allocates an array of type T in the arena.
   */
  template <typename T> [[nodiscard]] T *allocateArray(size_t count) {
    if (count == 0)
      return nullptr;
    return static_cast<T *>(allocate(count * sizeof(T), alignof(T)));
  }

  /**
   * @brief Constructs an array of type T in the arena.
   */
  template <typename T> [[nodiscard]] T *makeArray(size_t count) {
    if (count == 0)
      return nullptr;
    T *arr = allocateArray<T>(count);
    for (size_t i = 0; i < count; i++) {
      new (&arr[i]) T();
    }
    return arr;
  }

  /**
   * @brief Allocates raw memory in the arena.
   */
  [[nodiscard]] void *allocate(size_t size, size_t alignment) {
    size_t currentPtr =
        reinterpret_cast<size_t>(blocks_.back().get()) + currentOffset_;
    size_t padding = (alignment - (currentPtr % alignment)) % alignment;

    if (currentOffset_ + padding + size > blockSize_) {
      allocateBlock();
      currentOffset_ = 0;
      currentPtr = reinterpret_cast<size_t>(blocks_.back().get());
      padding = (alignment - (currentPtr % alignment)) % alignment;
    }

    currentOffset_ += padding;
    void *result = blocks_.back().get() + currentOffset_;
    currentOffset_ += size;
    return result;
  }

  /**
   * @brief Resets the arena, deallocating all managed memory.
   */
  void reset() {
    blocks_.clear();
    currentOffset_ = 0;
    allocateBlock();
  }

private:
  void allocateBlock() {
    blocks_.emplace_back(std::make_unique<uint8_t[]>(blockSize_));
    currentOffset_ = 0;
  }

  size_t blockSize_;
  std::vector<std::unique_ptr<uint8_t[]>> blocks_;
  size_t currentOffset_ = 0;
};

} // namespace druk::util
