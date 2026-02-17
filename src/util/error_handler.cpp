#include "druk/util/error_handler.hpp"
#include <format>
#include <iostream>

namespace druk::util {

std::string Diagnostic::toString(std::string_view /*sourceCode*/) const {
  return std::format("[{}:{}] {}: {}", location.line, location.column,
                     severity == DiagnosticsSeverity::Error ? "Error" : "Warning",
                     message);
}

void ErrorHandler::report(Diagnostic diagnostic) {
  if (diagnostic.severity == DiagnosticsSeverity::Error ||
      diagnostic.severity == DiagnosticsSeverity::Fatal) {
    errorCount_++;
  }
  diagnostics_.push_back(std::move(diagnostic));
}

void ErrorHandler::print(std::string_view sourceCode) const {
  for (const auto &diag : diagnostics_) {
    std::cerr << diag.toString(sourceCode) << "\n";
  }
}

} // namespace druk::util
