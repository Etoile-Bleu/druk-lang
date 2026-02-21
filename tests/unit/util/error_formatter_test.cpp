#include "druk/util/error_formatter.hpp"

#include <gtest/gtest.h>


using namespace druk::util;

TEST(ErrorFormatterTest, ExtractLine)
{
    std::string_view source = "line 1\nline 2\nline 3\nline 4";

    EXPECT_EQ(extractLine(source, 1), "line 1");
    EXPECT_EQ(extractLine(source, 2), "line 2");
    EXPECT_EQ(extractLine(source, 3), "line 3");
    EXPECT_EQ(extractLine(source, 4), "line 4");

    // Out of bounds
    EXPECT_EQ(extractLine(source, 0), "");
    EXPECT_EQ(extractLine(source, 5), "");
}

TEST(ErrorFormatterTest, ExtractLineTrailingNewline)
{
    std::string_view source = "single line\n";
    EXPECT_EQ(extractLine(source, 1), "single line");
    EXPECT_EQ(extractLine(source, 2), "");
}

TEST(ErrorFormatterTest, SeverityColors)
{
    EXPECT_EQ(getSeverityColor(DiagnosticsSeverity::Error), "\033[31m");
    EXPECT_EQ(getSeverityColor(DiagnosticsSeverity::Warning), "\033[33m");
    EXPECT_EQ(getSeverityColor(DiagnosticsSeverity::Note), "\033[36m");
}

TEST(ErrorFormatterTest, SeverityNames)
{
    EXPECT_EQ(getSeverityName(DiagnosticsSeverity::Error), "error");
    EXPECT_EQ(getSeverityName(DiagnosticsSeverity::Warning), "warning");
    EXPECT_EQ(getSeverityName(DiagnosticsSeverity::Note), "note");
}
