#pragma once

namespace druk::ir
{

enum class Opcode
{
    // Arithmetic
    Add,
    Sub,
    Mul,
    Div,
    Mod,

    // Comparison
    Equal,
    NotEqual,
    LessThan,
    LessEqual,
    GreaterThan,
    GreaterEqual,

    // Logical
    And,
    Or,
    Not,

    // Memory
    Load,
    Store,
    Alloca,

    // Aggregate
    BuildArray,
    IndexGet,
    IndexSet,
    Len,
    Push,
    Pop,
    Typeof,
    Keys,
    Values,
    Contains,

    // Control flow
    Branch,
    ConditionalBranch,
    Return,
    Call,
    DynamicCall,
    Print,

    // Type conversion
    IntToFloat,
    FloatToInt,
    Bitcast
};

}  // namespace druk::ir
