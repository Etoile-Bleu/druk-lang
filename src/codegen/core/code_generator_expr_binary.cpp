#include "druk/codegen/core/code_generator.h"
#include "druk/ir/ir_instruction.h"
#include "druk/ir/ir_type.h"
#include "druk/ir/ir_value.h"

namespace druk::codegen
{

void CodeGenerator::visitBinary(parser::ast::BinaryExpr* expr)
{
    visit(expr->left);
    auto* leftVal = lastValue_;
    visit(expr->right);
    auto* rightVal = lastValue_;
    if (!leftVal || !rightVal)
    {
        lastValue_ = nullptr;
        return;
    }
    ir::Instruction* inst = nullptr;
    switch (expr->token.type)
    {
        case lexer::TokenType::Plus:
            inst = builder_.createAdd(leftVal, rightVal);
            break;
        case lexer::TokenType::Minus:
            inst = builder_.createSub(leftVal, rightVal);
            break;
        case lexer::TokenType::Star:
            inst = builder_.createMul(leftVal, rightVal);
            break;
        case lexer::TokenType::Slash:
            inst = builder_.createDiv(leftVal, rightVal);
            break;
        case lexer::TokenType::EqualEqual:
            inst = builder_.createEqual(leftVal, rightVal);
            break;
        case lexer::TokenType::BangEqual:
            inst = builder_.createNotEqual(leftVal, rightVal);
            break;
        case lexer::TokenType::Less:
            inst = builder_.createLessThan(leftVal, rightVal);
            break;
        case lexer::TokenType::LessEqual:
            inst = builder_.createLessEqual(leftVal, rightVal);
            break;
        case lexer::TokenType::Greater:
            inst = builder_.createGreaterThan(leftVal, rightVal);
            break;
        case lexer::TokenType::GreaterEqual:
            inst = builder_.createGreaterEqual(leftVal, rightVal);
            break;
        default:
            lastValue_ = nullptr;
            return;
    }
    lastValue_ = inst;
}

void CodeGenerator::visitUnary(parser::ast::UnaryExpr* expr)
{
    visit(expr->right);
    lastValue_ = nullptr;
}

}  // namespace druk::codegen
