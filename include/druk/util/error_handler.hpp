#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace druk::util
{

/**
 * @brief Represents a location within a source file.
 */
struct SourceLocation
{
    uint32_t line;
    uint32_t column;
    uint32_t offset;
    uint32_t length;
};

/**
 * @brief Severity levels for diagnostics.
 */
enum class DiagnosticsSeverity
{
    Note,
    Warning,
    Error,
    Fatal
};

/**
 * @brief Represents a single diagnostic entry (error or warning).
 */
struct Diagnostic
{
    DiagnosticsSeverity severity;
    SourceLocation      location;
    std::string         message;
    std::string         suggestion;
};

/**
 * @brief ErrorHandler manages the reporting and tracking of diagnostics.
 */
class ErrorHandler
{
   public:
    void report(Diagnostic diagnostic);

    [[nodiscard]] bool hasErrors() const
    {
        return errorCount_ > 0;
    }
    [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const
    {
        return diagnostics_;
    }

    void print(std::string_view sourceCode, std::string_view fileName) const;

   private:
    std::vector<Diagnostic> diagnostics_;
    size_t                  errorCount_ = 0;
};

}  // namespace druk::util
