#include "druk/util/error_handler.hpp"

#include <iostream>
#include <string>

#include "druk/util/error_formatter.hpp"


namespace druk::util
{

void ErrorHandler::report(Diagnostic diagnostic)
{
    if (diagnostic.severity == DiagnosticsSeverity::Error ||
        diagnostic.severity == DiagnosticsSeverity::Fatal)
    {
        errorCount_++;
    }
    diagnostics_.push_back(std::move(diagnostic));
}

void ErrorHandler::print(std::string_view sourceCode, std::string_view fileName) const
{
    constexpr const char* Reset = "\033[0m";
    constexpr const char* Bold  = "\033[1m";

    for (const auto& diag : diagnostics_)
    {
        std::string color   = getSeverityColor(diag.severity);
        std::string sevName = getSeverityName(diag.severity);

        std::cerr << Bold << fileName << ":" << diag.location.line << ":" << diag.location.column
                  << ": " << Reset << color << sevName << ": " << Reset << Bold << diag.message
                  << Reset << "\n";

        std::string sourceLine = extractLine(sourceCode, diag.location.line);
        if (!sourceLine.empty())
        {
            std::string lineStr = std::to_string(diag.location.line);
            std::string margin  = std::string(lineStr.length(), ' ');

            std::cerr << " " << lineStr << " | " << sourceLine << "\n";

            std::cerr << " " << margin << " | ";

            std::string prefix;
            uint32_t    col = diag.location.column;
            if (col == 0 && diag.location.offset <= sourceCode.length())
            {
                size_t lineStart = diag.location.offset;
                while (lineStart > 0 && sourceCode[lineStart - 1] != '\n')
                {
                    lineStart--;
                }
                prefix =
                    std::string(sourceCode.substr(lineStart, diag.location.offset - lineStart));
            }
            else
            {
                uint32_t fill = col > 0 ? (col - 1) : 0;
                prefix        = std::string(fill, ' ');
            }

            for (char c : prefix)
            {
                if (c == '\t')
                {
                    std::cerr << '\t';
                }
                else if ((c & 0xC0) != 0x80)
                {
                    std::cerr << ' ';
                }
            }
            std::cerr << color << "^" << Reset << "\n";
        }

        if (!diag.suggestion.empty())
        {
            std::string margin = std::string(std::to_string(diag.location.line).length(), ' ');
            std::cerr << " " << margin << " | " << "\033[36mnote:\033[0m " << diag.suggestion
                      << "\n";
        }
    }
}

}  // namespace druk::util
