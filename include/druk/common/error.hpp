#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>


namespace druk {

struct SourceLocation {
  uint32_t line;
  uint32_t column;
  uint32_t offset;
  uint32_t length;
};

enum class ErrorLevel { Warning, Error, Fatal };

struct Error {
  ErrorLevel level;
  SourceLocation location;
  std::string message;
  std::string suggestion; // Optional fix suggestion

  [[nodiscard]] std::string to_string(std::string_view source_code) const;
};

class ErrorReporter {
public:
  void report(Error error);
  [[nodiscard]] bool has_errors() const { return error_count_ > 0; }
  [[nodiscard]] const std::vector<Error> &errors() const { return errors_; }

private:
  std::vector<Error> errors_;
  size_t error_count_ = 0;
};

} // namespace druk
