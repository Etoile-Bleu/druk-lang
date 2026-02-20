#pragma once

#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "druk/ir/ir_builder.h"
#include "druk/ir/ir_module.h"
#include "druk/parser/ast/ast.hpp"
#include "druk/parser/ast/visitor.hpp"
#include "druk/util/error_handler.hpp"

namespace druk::codegen
{

/**
 * @brief Generic code generator that translates AST to Druk IR.
 */
class CodeGenerator : public parser::ast::Visitor
{
   public:
    CodeGenerator(ir::Module& module, util::ErrorHandler& errors, std::string_view source);

    bool generate(const std::vector<parser::ast::Stmt*>& statements);

    // Visitor overrides for statements
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

    // Visitor overrides for expressions
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

    void visitBuiltinType(parser::ast::BuiltinType* type) override;
    void visitArrayType(parser::ast::ArrayType* type) override;
    void visitFunctionType(parser::ast::FunctionType* type) override;

   private:
    void visit(parser::ast::Stmt* stmt);
    void visit(parser::ast::Expr* expr);
    void visit(parser::ast::Type* type);

    ir::Module&         module_;
    ir::IRBuilder       builder_;
    util::ErrorHandler& errors_;
    std::string_view    source_;
    ir::Value*          lastValue_ = nullptr;  // Track result of last visited expression

    // Symbol tables
    std::unordered_map<std::string, ir::Value*>    variables_;  // Variable name -> alloca
    std::unordered_map<std::string, ir::Function*> functions_;  // Function name -> Function

    // Current function context (for parameter resolution)
    ir::Function* currentFunction_ = nullptr;
    uint32_t      lambdaCount_     = 0;
};

}  // namespace druk::codegen
