#include "type_checker.hpp"

namespace druk::semantic {

TypeChecker::TypeChecker(util::ErrorHandler& errors, SymbolTable& table, std::string_view source)
    : errors_(errors), table_(table), source_(source) {}

void TypeChecker::check(parser::ast::Node* node) {
    if (!node) return;
    if (auto* stmt = dynamic_cast<parser::ast::Stmt*>(node)) {
        visit(stmt);
    } else if (auto* expr = dynamic_cast<parser::ast::Expr*>(node)) {
        visit(expr);
    }
}

void TypeChecker::visit(parser::ast::Stmt* stmt) {
    if (!stmt) return;
    switch (stmt->kind) {
        case parser::ast::NodeKind::Block: {
            auto* block = static_cast<parser::ast::BlockStmt*>(stmt);
            for (uint32_t i = 0; i < block->count; ++i) {
                check(block->statements[i]);
            }
            break;
        }
        case parser::ast::NodeKind::Variable: {
            auto* var = static_cast<parser::ast::VarDecl*>(stmt);
            Type expected = Type::makeError();
            if (var->typeToken.type == lexer::TokenType::KwNumber) expected = Type::makeInt();
            else if (var->typeToken.type == lexer::TokenType::KwString) expected = Type::makeString();
            else if (var->typeToken.type == lexer::TokenType::KwBoolean) expected = Type::makeBool();
            
            if (var->initializer) {
                Type initType = visit(var->initializer);
                if (initType != expected && initType != Type::makeError()) {
                    util::Diagnostic diag;
                    diag.severity = util::DiagnosticsSeverity::Error;
                    diag.location = {var->initializer->token.line, 0, var->initializer->token.offset, var->initializer->token.length};
                    diag.message = "Type mismatch: cannot initialize " + typeToString(expected) + " with " + typeToString(initType);
                    errors_.report(diag);
                }
            }
            if (auto* sym = table_.resolve(std::string(var->name.text(source_)))) {
                sym->type = expected;
            }
            break;
        }
        case parser::ast::NodeKind::Function: {
            auto* func = static_cast<parser::ast::FuncDecl*>(stmt);
            table_.enterScope();
            for (uint32_t i = 0; i < func->paramCount; ++i) {
                 if (auto* sym = table_.resolve(std::string(func->params[i].text(source_)))) {
                    sym->type = Type::makeInt();
                 }
            }
            check(func->body);
            table_.exitScope();
            break;
        }
        case parser::ast::NodeKind::If: {
            auto* ifStmt = static_cast<parser::ast::IfStmt*>(stmt);
            if (visit(ifStmt->condition) != Type::makeBool()) {
                // report error
            }
            check(ifStmt->thenBranch);
            if (ifStmt->elseBranch) check(ifStmt->elseBranch);
            break;
        }
        case parser::ast::NodeKind::Loop: {
            auto* loop = static_cast<parser::ast::LoopStmt*>(stmt);
            if (visit(loop->condition) != Type::makeBool()) {
                // report error
            }
            check(loop->body);
            break;
        }
        default: break;
    }
}

Type TypeChecker::visit(parser::ast::Expr* expr) {
    if (!expr) return Type::makeVoid();
    
    switch (expr->kind) {
        case parser::ast::NodeKind::Literal: {
            auto* lit = static_cast<parser::ast::LiteralExpr*>(expr);
            if (lit->token.type == lexer::TokenType::Number) expr->type = Type::makeInt();
            else if (lit->token.type == lexer::TokenType::String) expr->type = Type::makeString();
            else expr->type = Type::makeBool();
            return expr->type;
        }
        case parser::ast::NodeKind::VariableExpr: {
            auto* var = static_cast<parser::ast::VariableExpr*>(expr);
            if (auto* sym = table_.resolve(std::string(var->name.text(source_)))) {
                expr->type = sym->type;
            } else {
                expr->type = Type::makeError();
            }
            return expr->type;
        }
        case parser::ast::NodeKind::Binary: {
            auto* bin = static_cast<parser::ast::BinaryExpr*>(expr);
            Type left = visit(bin->left);
            Type right = visit(bin->right);
            if (left == Type::makeInt() && right == Type::makeInt()) expr->type = Type::makeInt();
            else expr->type = Type::makeError();
            return expr->type;
        }
        case parser::ast::NodeKind::Grouping: {
            auto* group = static_cast<parser::ast::GroupingExpr*>(expr);
            expr->type = visit(group->expression);
            return expr->type;
        }
        case parser::ast::NodeKind::Unary: {
            auto* un = static_cast<parser::ast::UnaryExpr*>(expr);
            expr->type = visit(un->right);
            return expr->type;
        }
        default: break;
    }
    return Type::makeError();
}

} // namespace druk::semantic
