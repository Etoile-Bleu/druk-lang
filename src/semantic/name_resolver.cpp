#include "name_resolver.hpp"

#include "druk/parser/ast/lambda.hpp"
#include "druk/parser/ast/stmt.hpp"

namespace druk::semantic
{

NameResolver::NameResolver(util::ErrorHandler& errors, SymbolTable& table, std::string_view source)
    : errors_(errors), table_(table), source_(source)
{
}

void NameResolver::resolve(parser::ast::Node* node)
{
    if (!node)
        return;
    if (auto* stmt = dynamic_cast<parser::ast::Stmt*>(node))
    {
        visit(stmt);
    }
    else if (auto* expr = dynamic_cast<parser::ast::Expr*>(node))
    {
        visit(expr);
    }
}

void NameResolver::defineSymbols(const std::vector<parser::ast::Stmt*>& statements)
{
    for (auto* stmt : statements)
    {
        if (stmt->kind == parser::ast::NodeKind::Function)
        {
            auto* func = static_cast<parser::ast::FuncDecl*>(stmt);
            table_.define(std::string(func->name.text(source_)), {func->name, Type::makeInt()});
        }
    }
}

void NameResolver::resolveBodies(const std::vector<parser::ast::Stmt*>& statements)
{
    for (auto* stmt : statements)
    {
        resolve(stmt);
    }
}

void NameResolver::visit(parser::ast::Stmt* stmt)
{
    if (!stmt)
        return;
    switch (stmt->kind)
    {
        case parser::ast::NodeKind::Block:
        {
            auto* block = static_cast<parser::ast::BlockStmt*>(stmt);
            table_.enterScope();
            // Nested two-pass for blocks
            defineSymbols({block->statements, block->statements + block->count});
            for (uint32_t i = 0; i < block->count; ++i)
            {
                resolve(block->statements[i]);
            }
            table_.exitScope();
            break;
        }
        case parser::ast::NodeKind::Function:
        {
            auto* func = static_cast<parser::ast::FuncDecl*>(stmt);
            // Function name already defined by caller's defineSymbols() pass

            table_.enterScope();
            for (uint32_t i = 0; i < func->paramCount; ++i)
            {
                table_.define(std::string(func->params[i].name.text(source_)),
                              {func->params[i].name, Type::makeInt()});
            }
            // Functions can be nested? If so, we'd need another pass inside.
            // For now, assume top-level or single level nesting.
            if (func->body->kind == parser::ast::NodeKind::Block)
            {
                auto* block = static_cast<parser::ast::BlockStmt*>(func->body);
                defineSymbols({block->statements, block->statements + block->count});
            }
            resolve(func->body);
            table_.exitScope();
            break;
        }
        case parser::ast::NodeKind::Variable:
        {
            auto* var = static_cast<parser::ast::VarDecl*>(stmt);
            table_.define(std::string(var->name.text(source_)), {var->name, Type::makeError()});
            resolve(var->initializer);
            break;
        }
        case parser::ast::NodeKind::If:
        {
            auto* ifStmt = static_cast<parser::ast::IfStmt*>(stmt);
            resolve(ifStmt->condition);
            resolve(ifStmt->thenBranch);
            resolve(ifStmt->elseBranch);
            break;
        }
        case parser::ast::NodeKind::Loop:
        {
            auto* loop = static_cast<parser::ast::LoopStmt*>(stmt);
            resolve(loop->condition);
            resolve(loop->body);
            break;
        }
        case parser::ast::NodeKind::While:
        {
            auto* ws = static_cast<parser::ast::WhileStmt*>(stmt);
            resolve(ws->condition);
            resolve(ws->body);
            break;
        }
        case parser::ast::NodeKind::For:
        {
            auto* fs = static_cast<parser::ast::ForStmt*>(stmt);
            table_.enterScope();
            if (fs->init)
                resolve(fs->init);
            if (fs->condition)
                resolve(fs->condition);
            if (fs->step)
                resolve(fs->step);
            resolve(fs->body);
            table_.exitScope();
            break;
        }
        case parser::ast::NodeKind::Return:
        {
            auto* ret = static_cast<parser::ast::ReturnStmt*>(stmt);
            resolve(ret->value);
            break;
        }
        case parser::ast::NodeKind::Print:
        {
            auto* print = static_cast<parser::ast::PrintStmt*>(stmt);
            resolve(print->expression);
            break;
        }
        case parser::ast::NodeKind::ExpressionStmt:
        {
            auto* exprStmt = static_cast<parser::ast::ExpressionStmt*>(stmt);
            resolve(exprStmt->expression);
            break;
        }
        default:
            break;
    }
}

void NameResolver::visit(parser::ast::Expr* expr)
{
    if (!expr)
        return;
    switch (expr->kind)
    {
        case parser::ast::NodeKind::Binary:
        {
            auto* bin = static_cast<parser::ast::BinaryExpr*>(expr);
            resolve(bin->left);
            resolve(bin->right);
            break;
        }
        case parser::ast::NodeKind::Unary:
        {
            auto* un = static_cast<parser::ast::UnaryExpr*>(expr);
            resolve(un->right);
            break;
        }
        case parser::ast::NodeKind::Call:
        {
            auto* call = static_cast<parser::ast::CallExpr*>(expr);
            resolve(call->callee);
            for (uint32_t i = 0; i < call->argCount; ++i)
            {
                resolve(static_cast<parser::ast::Expr*>(call->args[i]));
            }
            break;
        }
        case parser::ast::NodeKind::Grouping:
        {
            auto* group = static_cast<parser::ast::GroupingExpr*>(expr);
            resolve(group->expression);
            break;
        }
        case parser::ast::NodeKind::VariableExpr:
        {
            auto* var = static_cast<parser::ast::VariableExpr*>(expr);
            if (!table_.resolve(std::string(var->name.text(source_))))
            {
                util::Diagnostic diag;
                diag.severity = util::DiagnosticsSeverity::Error;
                diag.location = {var->name.line, var->name.column, var->name.offset,
                                 var->name.length};
                diag.message = "Undeclared variable '" + std::string(var->name.text(source_)) + "'";
                errors_.report(diag);
            }
            break;
        }
        case parser::ast::NodeKind::Assignment:
        {
            auto* assign = static_cast<parser::ast::AssignmentExpr*>(expr);
            resolve(assign->target);
            resolve(assign->value);
            break;
        }
        case parser::ast::NodeKind::Logical:
        {
            auto* log = static_cast<parser::ast::LogicalExpr*>(expr);
            resolve(log->left);
            resolve(log->right);
            break;
        }
        case parser::ast::NodeKind::ArrayLiteral:
        {
            auto* arr = static_cast<parser::ast::ArrayLiteralExpr*>(expr);
            for (uint32_t i = 0; i < arr->count; ++i) resolve(arr->elements[i]);
            break;
        }
        case parser::ast::NodeKind::Index:
        {
            auto* idx = static_cast<parser::ast::IndexExpr*>(expr);
            resolve(idx->array);
            resolve(idx->index);
            break;
        }
        case parser::ast::NodeKind::StructLiteral:
        {
            auto* st = static_cast<parser::ast::StructLiteralExpr*>(expr);
            for (uint32_t i = 0; i < st->fieldCount; ++i) resolve(st->fieldValues[i]);
            break;
        }
        case parser::ast::NodeKind::MemberAccess:
        {
            auto* mem = static_cast<parser::ast::MemberAccessExpr*>(expr);
            resolve(mem->object);
            break;
        }
        case parser::ast::NodeKind::Lambda:
        {
            auto* lambda = static_cast<parser::ast::LambdaExpr*>(expr);
            table_.enterScope();
            for (uint32_t i = 0; i < lambda->paramCount; ++i)
            {
                table_.define(std::string(lambda->params[i].name.text(source_)),
                              {lambda->params[i].name, Type::makeError()});
            }
            resolve(lambda->body);
            table_.exitScope();
            break;
        }
        case parser::ast::NodeKind::Literal:
            break;
        default:
            break;
    }
}

}  // namespace druk::semantic
