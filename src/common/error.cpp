#include "druk/common/error.hpp"
#include <format>
#include <iostream>

namespace druk {

std::string Error::to_string(std::string_view source_code) const {
  // Simple formatting for now.
  // In a real compiler, we'd print the line snippet with a caret.
  return std::format("[{}:{}] {}: {}", location.line, location.column,
                     level == ErrorLevel::Error ? "Error" : "Warning", message);
}

void ErrorReporter::report(Error error) {
  if (error.level == ErrorLevel::Error || error.level == ErrorLevel::Fatal) {
    error_count_++;
  }
  errors_.push_back(std::move(error));
}

} // namespace druk
