#ifndef KOOLANG_AST_AST_H
#define KOOLANG_AST_AST_H

#include "TokenList.h"
#include "ast/Node.h"
#include <vector>

namespace ast {

struct Ast {
    std::vector<Tag> NodeTags;
    std::vector<Index> NodeTokens;
    std::vector<Node> Nodes;

    std::vector<Index> Meta;

    std::vector<Index> Imports;
    std::vector<Index> Top;
    TokenList Tokens;
};

}

#endif // KOOLANG_AST_AST_H
