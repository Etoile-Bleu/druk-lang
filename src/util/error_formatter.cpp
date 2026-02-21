#include "druk/util/error_formatter.hpp"

namespace druk::util
{

std::string getSeverityColor(DiagnosticsSeverity severity)
{
    switch (severity)
    {
        case DiagnosticsSeverity::Note:
            return "\033[36m";
        case DiagnosticsSeverity::Warning:
            return "\033[33m";
        case DiagnosticsSeverity::Error:
            return "\033[31m";
        case DiagnosticsSeverity::Fatal:
            return "\033[1;31m";
    }
    return "";
}

std::string getSeverityName(DiagnosticsSeverity severity)
{
    switch (severity)
    {
        case DiagnosticsSeverity::Note:
            return "note";
        case DiagnosticsSeverity::Warning:
            return "warning";
        case DiagnosticsSeverity::Error:
            return "error";
        case DiagnosticsSeverity::Fatal:
            return "fatal error";
    }
    return "";
}

std::string extractLine(std::string_view sourceCode, uint32_t lineNum)
{
    uint32_t currentLine = 1;
    size_t   startPos    = 0;
    while (currentLine < lineNum && startPos < sourceCode.length())
    {
        size_t nextNL = sourceCode.find('\n', startPos);
        if (nextNL == std::string_view::npos)
        {
            break;
        }
        startPos = nextNL + 1;
        currentLine++;
    }

    if (currentLine != lineNum || startPos >= sourceCode.length())
    {
        return "";
    }

    size_t endPos = sourceCode.find('\n', startPos);
    if (endPos == std::string_view::npos)
    {
        endPos = sourceCode.length();
    }

    return std::string(sourceCode.substr(startPos, endPos - startPos));
}

}  // namespace druk::util
