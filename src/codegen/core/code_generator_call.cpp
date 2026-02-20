/**
 * @file code_generator_call.cpp
 * @brief Function call IR generation.
 */

#include <string>

#include "druk/codegen/core/code_generator.h"

namespace druk::codegen
{

static std::string strip_trailing_tsheg(std::string name)
{
    const char tsheg[] = "\xE0\xBC\x8B";  // U+0F0B
    if (name.size() >= 3 && name.compare(name.size() - 3, 3, tsheg) == 0)
    {
        name.resize(name.size() - 3);
    }
    return name;
}

void CodeGenerator::visitCall(parser::ast::CallExpr* expr)
{
    ir::Function* func          = nullptr;
    ir::Value*    dynamicCallee = nullptr;
    std::string   funcName;

    auto* varExpr = dynamic_cast<parser::ast::VariableExpr*>(expr->callee);
    if (varExpr)
    {
        funcName         = std::string(varExpr->name.text(source_));
        auto builtinName = strip_trailing_tsheg(funcName);
        if (builtinName == "ཚད")
        {
            if (expr->argCount != 1)
            {
                util::Diagnostic diag{util::DiagnosticsSeverity::Error,
                                      {expr->token.line, 0, expr->token.offset, expr->token.length},
                                      "Call to '" + funcName + "' expects 1 argument, got " +
                                          std::to_string(expr->argCount) + ".",
                                      ""};
                errors_.report(std::move(diag));
                lastValue_ = nullptr;
                return;
            }

            auto* argExpr = static_cast<parser::ast::Expr*>(expr->args[0]);
            visit(argExpr);
            if (!lastValue_)
            {
                util::Diagnostic diag{util::DiagnosticsSeverity::Error,
                                      {expr->token.line, 0, expr->token.offset, expr->token.length},
                                      "Argument 1 to '" + funcName + "' could not be evaluated.",
                                      ""};
                errors_.report(std::move(diag));
                lastValue_ = nullptr;
                return;
            }

            lastValue_ = builder_.createLen(lastValue_);
            return;
        }

        auto it = functions_.find(funcName);
        if (it != functions_.end())
        {
            func = it->second;
        }
        else
        {
            auto itVar = variables_.find(funcName);
            if (itVar != variables_.end())
            {
                dynamicCallee = builder_.createLoad(itVar->second);
            }
        }
    }
    else
    {
        visit(expr->callee);
        dynamicCallee = lastValue_;
    }

    if (!func && !dynamicCallee)
    {
        lastValue_ = nullptr;
        return;
    }

    uint32_t expected = func ? func->getParameterCount() : expr->argCount;
    // We can't easily check param count for dynamic calls at compile time yet.

    std::vector<ir::Value*> args;
    args.reserve(expr->argCount);
    for (uint32_t i = 0; i < expr->argCount; ++i)
    {
        auto* argExpr = static_cast<parser::ast::Expr*>(expr->args[i]);
        visit(argExpr);
        if (!lastValue_)
        {
            util::Diagnostic diag{
                util::DiagnosticsSeverity::Error,
                {expr->token.line, 0, expr->token.offset, expr->token.length},
                "Argument " + std::to_string(i + 1) + " to call could not be evaluated.",
                ""};
            errors_.report(std::move(diag));
            lastValue_ = nullptr;
            return;
        }
        args.push_back(lastValue_);
    }

    if (func)
    {
        if (expr->argCount != expected)
        {
            util::Diagnostic diag{util::DiagnosticsSeverity::Error,
                                  {expr->token.line, 0, expr->token.offset, expr->token.length},
                                  "Call to '" + funcName + "' expects " + std::to_string(expected) +
                                      " arguments, got " + std::to_string(expr->argCount) + ".",
                                  ""};
            errors_.report(std::move(diag));
            lastValue_ = nullptr;
            return;
        }
        lastValue_ = builder_.createCall(func, args);
    }
    else
    {
        lastValue_ = builder_.createDynamicCall(dynamicCallee, args);
    }
}

}  // namespace druk::codegen
