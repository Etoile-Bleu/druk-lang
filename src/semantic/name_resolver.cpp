#include "name_resolver.hpp"

namespace druk::semantic {

NameResolver::NameResolver(util::ErrorHandler& errors, SymbolTable& table, std::string_view source)
    : errors_(errors), table_(table), source_(source) {}

void NameResolver::resolve(parser::ast::Node* node) {
    if (!node) return;
    if (auto* stmt = dynamic_cast<parser::ast::Stmt*>(node)) {
        visit(stmt);
    } else if (auto* expr = dynamic_cast<parser::ast::Expr*>(node)) {
        visit(expr);
    }
}

void NameResolver::visit(parser::ast::Stmt* stmt) {
    if (!stmt) return;
    switch (stmt->kind) {
        case parser::ast::NodeKind::Block: {
            auto* block = static_cast<parser::ast::BlockStmt*>(stmt);
            table_.enterScope();
            for (uint32_t i = 0; i < block->count; ++i) {
                resolve(block->statements[i]);
            }
            table_.exitScope();
            break;
        }
        case parser::ast::NodeKind::Function: {
            auto* func = static_cast<parser::ast::FuncDecl*>(stmt);
            // Define in current (presumably global) scope
            if (!table_.define(std::string(func->name.text(source_)), {func->name, Type::makeInt()})) {
                // Warning or error? Function already defined.
            }
            
            table_.enterScope();
            for (uint32_t i = 0; i < func->paramCount; ++i) {
                table_.define(std::string(func->params[i].text(source_)), {func->params[i], Type::makeInt()});
            }
            resolve(func->body);
            table_.exitScope();
            break;
        }
        case parser::ast::NodeKind::Variable: {
            auto* var = static_cast<parser::ast::VarDecl*>(stmt);
            table_.define(std::string(var->name.text(source_)), {var->name, Type::makeError()});
            resolve(var->initializer);
            break;
        }
        case parser::ast::NodeKind::If: {
            auto* ifStmt = static_cast<parser::ast::IfStmt*>(stmt);
            resolve(ifStmt->condition);
            resolve(ifStmt->thenBranch);
            resolve(ifStmt->elseBranch);
            break;
        }
        case parser::ast::NodeKind::Loop: {
            auto* loop = static_cast<parser::ast::LoopStmt*>(stmt);
            resolve(loop->condition);
            resolve(loop->body);
            break;
        }
        case parser::ast::NodeKind::Return: {
            auto* ret = static_cast<parser::ast::ReturnStmt*>(stmt);
            resolve(ret->value);
            break;
        }
        case parser::ast::NodeKind::Print: {
            auto* print = static_cast<parser::ast::PrintStmt*>(stmt);
            resolve(print->expression);
            break;
        }
        case parser::ast::NodeKind::ExpressionStmt: {
            auto* exprStmt = static_cast<parser::ast::ExpressionStmt*>(stmt);
            resolve(exprStmt->expression);
            break;
        }
        default: break;
    }
}

void NameResolver::visit(parser::ast::Expr* expr) {
    if (!expr) return;
    switch (expr->kind) {
        case parser::ast::NodeKind::Binary: {
            auto* bin = static_cast<parser::ast::BinaryExpr*>(expr);
            resolve(bin->left);
            resolve(bin->right);
            break;
        }
        case parser::ast::NodeKind::Unary: {
            auto* un = static_cast<parser::ast::UnaryExpr*>(expr);
            resolve(un->right);
            break;
        }
        case parser::ast::NodeKind::Call: {
            auto* call = static_cast<parser::ast::CallExpr*>(expr);
            resolve(call->callee);
            for (uint32_t i = 0; i < call->argCount; ++i) {
                resolve(static_cast<parser::ast::Expr*>(call->args[i]));
            }
            break;
        }
        case parser::ast::NodeKind::Grouping: {
            auto* group = static_cast<parser::ast::GroupingExpr*>(expr);
            resolve(group->expression);
            break;
        }
        case parser::ast::NodeKind::VariableExpr:
        case parser::ast::NodeKind::Literal:
            break;
        default: break;
    }
}

} // namespace druk::semantic
