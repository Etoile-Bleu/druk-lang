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
    if (expr->token.type == lexer::TokenType::Plus)
    {
        bool leftStr  = leftVal->getType()->getID() == ir::TypeID::Pointer;
        bool rightStr = rightVal->getType()->getID() == ir::TypeID::Pointer;

        if (leftStr || rightStr)
        {
            if (!leftStr) leftVal = builder_.createToString(leftVal);
            if (!rightStr) rightVal = builder_.createToString(rightVal);
            inst = builder_.createStringConcat(leftVal, rightVal);
        }
        else
        {
            inst = builder_.createAdd(leftVal, rightVal);
        }
    }
    else if (expr->token.type == lexer::TokenType::Minus)
    {
        inst = builder_.createSub(leftVal, rightVal);
    }
    else if (expr->token.type == lexer::TokenType::Star)
    {
        inst = builder_.createMul(leftVal, rightVal);
    }
    else if (expr->token.type == lexer::TokenType::Slash)
    {
        inst = builder_.createDiv(leftVal, rightVal);
    }
    else if (expr->token.type == lexer::TokenType::EqualEqual)
    {
        inst = builder_.createEqual(leftVal, rightVal);
    }
    else if (expr->token.type == lexer::TokenType::BangEqual)
    {
        inst = builder_.createNotEqual(leftVal, rightVal);
    }
    else if (expr->token.type == lexer::TokenType::Less)
    {
        inst = builder_.createLessThan(leftVal, rightVal);
    }
    else if (expr->token.type == lexer::TokenType::LessEqual)
    {
        inst = builder_.createLessEqual(leftVal, rightVal);
    }
    else if (expr->token.type == lexer::TokenType::Greater)
    {
        inst = builder_.createGreaterThan(leftVal, rightVal);
    }
    else if (expr->token.type == lexer::TokenType::GreaterEqual)
    {
        inst = builder_.createGreaterEqual(leftVal, rightVal);
    }
    else if (expr->token.type == lexer::TokenType::And)
    {
        inst = builder_.createAnd(leftVal, rightVal);
    }
    else if (expr->token.type == lexer::TokenType::Or)
    {
        inst = builder_.createOr(leftVal, rightVal);
    }
    else
    {
        lastValue_ = nullptr;
        return;
    }
    lastValue_ = inst;
}

void CodeGenerator::visitUnary(parser::ast::UnaryExpr* expr)
{
    visit(expr->right);
    auto* val = lastValue_;
    if (!val) return;

    ir::Instruction* inst = nullptr;
    if (expr->token.type == lexer::TokenType::Minus)
    {
        inst = builder_.createNeg(val);
    }
    else if (expr->token.type == lexer::TokenType::Bang)
    {
        inst = builder_.createNot(val);
    }
    lastValue_ = inst;
}

void CodeGenerator::visitLogical(parser::ast::LogicalExpr* expr)
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

    if (expr->token.type == lexer::TokenType::And)
    {
        lastValue_ = builder_.createAnd(leftVal, rightVal);
    }
    else if (expr->token.type == lexer::TokenType::Or)
    {
        lastValue_ = builder_.createOr(leftVal, rightVal);
    }
    else
    {
        lastValue_ = nullptr;
    }
}

}  // namespace druk::codegen
