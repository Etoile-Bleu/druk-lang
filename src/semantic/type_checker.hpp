#pragma once

#include "druk/parser/ast/visitor.hpp"
#include "druk/semantic/symbol_table.hpp"
#include "druk/util/error_handler.hpp"

namespace druk::ir
{
class Function;
}

namespace druk::semantic
{

class TypeChecker : public parser::ast::Visitor
{
   public:
    TypeChecker(util::ErrorHandler& errors, SymbolTable& table, std::string_view source);

    void check(parser::ast::Node* node);
    Type analyze(parser::ast::Expr* expr);
    Type evaluate(parser::ast::Type* type);

    void visit(parser::ast::Stmt* stmt) override;
    void visit(parser::ast::Expr* expr) override;
    void visit(parser::ast::Type* type) override;

    void visitBlock(parser::ast::BlockStmt* stmt) override;
    void visitIf(parser::ast::IfStmt* stmt) override;
    void visitLoop(parser::ast::LoopStmt* stmt) override;
    void visitWhile(parser::ast::WhileStmt* stmt) override;
    void visitFor(parser::ast::ForStmt* stmt) override;
    void visitReturn(parser::ast::ReturnStmt* stmt) override;
    void visitPrint(parser::ast::PrintStmt* stmt) override;
    void visitExprStmt(parser::ast::ExpressionStmt* stmt) override;
    void visitVar(parser::ast::VarDecl* stmt) override;
    void visitFunc(parser::ast::FuncDecl* stmt) override;
    void visitMatch(parser::ast::MatchStmt* stmt) override;

    void visitBinary(parser::ast::BinaryExpr* expr) override;
    void visitUnary(parser::ast::UnaryExpr* expr) override;
    void visitLiteral(parser::ast::LiteralExpr* expr) override;
    void visitVariable(parser::ast::VariableExpr* expr) override;
    void visitGrouping(parser::ast::GroupingExpr* expr) override;
    void visitAssignment(parser::ast::AssignmentExpr* expr) override;
    void visitCall(parser::ast::CallExpr* expr) override;
    void visitLogical(parser::ast::LogicalExpr* expr) override;
    void visitArrayLiteral(parser::ast::ArrayLiteralExpr* expr) override;
    void visitIndex(parser::ast::IndexExpr* expr) override;
    void visitStructLiteral(parser::ast::StructLiteralExpr* expr) override;
    void visitMemberAccess(parser::ast::MemberAccessExpr* expr) override;
    void visitLambda(parser::ast::LambdaExpr* expr) override;
    void visitInterpolatedStringExpr(parser::ast::InterpolatedStringExpr* expr) override;
    void visitUnwrapExpr(parser::ast::UnwrapExpr* expr) override;

    void visitBuiltinType(parser::ast::BuiltinType* type) override;
    void visitArrayType(parser::ast::ArrayType* type) override;
    void visitFunctionType(parser::ast::FunctionType* type) override;
    void visitOptionType(parser::ast::OptionType* type) override;

   private:
    util::ErrorHandler& errors_;
    SymbolTable&        table_;
    std::string_view    source_;
    Type                currentType_ = Type::makeError();

    ir::Function* currentFunction_ = nullptr;
    uint32_t      lambdaCount_     = 0;

    void error(const lexer::Token& token, std::string message)
    {
        util::Diagnostic diag;
        diag.severity = util::DiagnosticsSeverity::Error;
        diag.location = {token.line, token.column, token.offset, token.length};
        diag.message  = std::move(message);
        errors_.report(diag);
    }

    void warn(const lexer::Token& token, std::string message)
    {
        util::Diagnostic diag;
        diag.severity = util::DiagnosticsSeverity::Warning;
        diag.location = {token.line, token.column, token.offset, token.length};
        diag.message  = std::move(message);
        errors_.report(diag);
    }
};

}  // namespace druk::semantic
