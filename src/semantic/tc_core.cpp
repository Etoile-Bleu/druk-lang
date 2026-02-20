#include "druk/parser/ast/expr.hpp"
#include "druk/parser/ast/stmt.hpp"
#include "type_checker.hpp"


namespace druk::semantic
{

TypeChecker::TypeChecker(util::ErrorHandler& errors, SymbolTable& table, std::string_view source)
    : errors_(errors), table_(table), source_(source)
{
}

void TypeChecker::check(parser::ast::Node* node)
{
    if (!node)
        return;
    node->accept(this);
}

Type TypeChecker::analyze(parser::ast::Expr* expr)
{
    if (!expr)
        return Type::makeVoid();
    expr->accept(this);
    return currentType_;
}

void TypeChecker::visit(parser::ast::Stmt* stmt)
{
    check(stmt);
}
void TypeChecker::visit(parser::ast::Expr* expr)
{
    analyze(expr);
}

}  // namespace druk::semantic
