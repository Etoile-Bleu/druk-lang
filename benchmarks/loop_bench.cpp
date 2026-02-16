#include <chrono>
#include <cstdint>
#include <iostream>

int main() {
  constexpr std::uint64_t n = 10000000;
  volatile std::uint64_t sum = 0;

  auto start = std::chrono::high_resolution_clock::now();
  for (std::uint64_t i = 0; i < n; ++i) {
    sum += 1;
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> ms = end - start;

  std::cout << sum << "\n";
  std::cout << "elapsed_ms=" << ms.count() << "\n";
  return 0;
}
