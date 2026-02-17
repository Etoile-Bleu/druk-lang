/**
 * @file code_generator_expr.cpp
 * @brief Expression IR generation: binary ops, unary ops, literals, variables, assignments.
 */

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
    auto* rightVal = lastValue_;

    if (!rightVal)
    {
        lastValue_ = nullptr;
        return;
    }

    // Unary operations not yet fully implemented in IR
    // For now, just return nullptr
    lastValue_ = nullptr;
}

void CodeGenerator::visitLiteral(parser::ast::LiteralExpr* expr)
{
    if (expr->literalValue.isInt())
    {
        lastValue_ = new ir::ConstantInt(expr->literalValue.asInt(), ir::Type::getInt64Ty());
    }
    else if (expr->literalValue.isBool())
    {
        lastValue_ = new ir::ConstantBool(expr->literalValue.asBool(), ir::Type::getBoolTy());
    }
    else if (expr->literalValue.isString())
    {
        auto charPtrTy = std::make_shared<ir::PointerType>(ir::Type::getInt8Ty());
        lastValue_ = new ir::ConstantString(std::string(expr->literalValue.asString()), charPtrTy);
    }
    else
    {
        lastValue_ = nullptr;
    }
}

void CodeGenerator::visitVariable(parser::ast::VariableExpr* expr)
{
    auto name = std::string(expr->name.text(source_));
    auto it = variables_.find(name);
    if (it != variables_.end())
    {
        lastValue_ = builder_.createLoad(it->second);
    }
    else
    {
        lastValue_ = nullptr;
    }
}

void CodeGenerator::visitAssignment(parser::ast::AssignmentExpr* expr)
{
    visit(expr->value);
    auto* val = lastValue_;

    if (!val)
    {
        lastValue_ = nullptr;
        return;
    }

    // Target should be a VariableExpr
    if (auto* varExpr = dynamic_cast<parser::ast::VariableExpr*>(expr->target))
    {
        auto name = std::string(varExpr->name.text(source_));
        auto it = variables_.find(name);
        if (it != variables_.end())
        {
            builder_.createStore(val, it->second);
            lastValue_ = val;
        }
        else
        {
            lastValue_ = nullptr;
        }
    }
    else
    {
        lastValue_ = nullptr;
    }
}

}  // namespace druk::codegen
