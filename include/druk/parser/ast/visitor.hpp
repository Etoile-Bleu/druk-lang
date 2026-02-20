#pragma once

namespace druk::parser::ast
{

struct BlockStmt;
struct IfStmt;
struct LoopStmt;
struct WhileStmt;
struct ForStmt;
struct ReturnStmt;
struct PrintStmt;
struct ExpressionStmt;
struct VarDecl;
struct FuncDecl;
struct MatchStmt;

struct BinaryExpr;
struct UnaryExpr;
struct LiteralExpr;
struct VariableExpr;
struct GroupingExpr;
struct AssignmentExpr;
struct CallExpr;
struct LogicalExpr;
struct ArrayLiteralExpr;
struct IndexExpr;
struct StructLiteralExpr;
struct MemberAccessExpr;
struct LambdaExpr;

struct Stmt;
struct Expr;
struct Type;

struct BuiltinType;
struct ArrayType;
struct FunctionType;

class Visitor
{
   public:
    virtual ~Visitor() = default;

    virtual void visit(Stmt* stmt) = 0;
    virtual void visit(Expr* expr) = 0;
    virtual void visit(Type* type) = 0;

    virtual void visitBlock(BlockStmt* stmt)         = 0;
    virtual void visitIf(IfStmt* stmt)               = 0;
    virtual void visitLoop(LoopStmt* stmt)           = 0;
    virtual void visitWhile(WhileStmt* stmt)         = 0;
    virtual void visitFor(ForStmt* stmt)             = 0;
    virtual void visitReturn(ReturnStmt* stmt)       = 0;
    virtual void visitPrint(PrintStmt* stmt)         = 0;
    virtual void visitExprStmt(ExpressionStmt* stmt) = 0;
    virtual void visitVar(VarDecl* stmt)             = 0;
    virtual void visitFunc(FuncDecl* stmt)           = 0;
    virtual void visitMatch(MatchStmt* stmt)         = 0;

    virtual void visitBinary(BinaryExpr* expr)               = 0;
    virtual void visitUnary(UnaryExpr* expr)                 = 0;
    virtual void visitLiteral(LiteralExpr* expr)             = 0;
    virtual void visitVariable(VariableExpr* expr)           = 0;
    virtual void visitGrouping(GroupingExpr* expr)           = 0;
    virtual void visitAssignment(AssignmentExpr* expr)       = 0;
    virtual void visitCall(CallExpr* expr)                   = 0;
    virtual void visitLogical(LogicalExpr* expr)             = 0;
    virtual void visitArrayLiteral(ArrayLiteralExpr* expr)   = 0;
    virtual void visitIndex(IndexExpr* expr)                 = 0;
    virtual void visitStructLiteral(StructLiteralExpr* expr) = 0;
    virtual void visitMemberAccess(MemberAccessExpr* expr)   = 0;
    virtual void visitLambda(LambdaExpr* expr)               = 0;

    virtual void visitBuiltinType(BuiltinType* type)   = 0;
    virtual void visitArrayType(ArrayType* type)       = 0;
    virtual void visitFunctionType(FunctionType* type) = 0;
};

}  // namespace druk::parser::ast
