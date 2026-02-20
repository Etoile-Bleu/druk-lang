#pragma once

#include <cstdint>

namespace druk::parser::ast
{

enum class NodeKind : uint8_t
{
    Function,
    Variable,

    Block,
    If,
    Loop,
    While,
    For,
    Match,
    Return,
    Print,
    ExpressionStmt,

    Binary,
    Unary,
    Literal,
    Grouping,
    VariableExpr,
    Call,
    Assignment,
    Logical,
    ArrayLiteral,
    Index,
    StructLiteral,
    MemberAccess,
    Lambda,
    BuiltinType,
    ArrayType,
    FunctionType,
    InterpolatedStringExpr,
    OptionType,
    UnwrapExpr
};

}
