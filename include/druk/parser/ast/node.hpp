#pragma once

#include "druk/lexer/token.hpp"
#include "druk/parser/ast/kinds.hpp"

namespace druk::parser::ast
{

class Visitor;

struct Node
{
    virtual ~Node()                 = default;
    virtual void accept(Visitor* v) = 0;

    NodeKind     kind;
    lexer::Token token;
};

}  // namespace druk::parser::ast
