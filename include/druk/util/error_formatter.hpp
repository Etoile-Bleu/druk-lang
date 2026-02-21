#pragma once

#include <string>
#include <string_view>

#include "druk/util/error_handler.hpp"


namespace druk::util
{

std::string getSeverityColor(DiagnosticsSeverity severity);
std::string getSeverityName(DiagnosticsSeverity severity);
std::string extractLine(std::string_view sourceCode, uint32_t lineNum);

}  // namespace druk::util
