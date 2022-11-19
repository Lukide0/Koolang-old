#ifndef KOOLANG_PARSER_H
#define KOOLANG_PARSER_H

#include <initializer_list>
#include <sstream>
#include <vector>

#include "File.h"
#include "Vis.h"
#include "ast/Ast.h"
#include "util/Index.h"

namespace ast {

class Parser {

public:
    Parser(File& file, std::string_view filepath);
    Ast Parse();

private:
    Ast m_ast;
    std::string_view m_filepath;
    std::string_view m_sourceCode;
    File& m_file;

    std::vector<Index> m_cache;

    // cached
    Index m_docTok;
    Index m_vis;

    //-- Parsing: Statements --//
    Index BlockStmt();
    Index ImportStmt();
    Index ConstantStmt();
    Index VarStmt();
    Index StaticStmt();
    Index FnStmt();
    Index VariantStmt();
    Index EnumStmt();
    Index StructStmt();
    Index TraitStmt();
    Index ImplStmt();
    Index IfStmt();
    Index ForStmt();
    Index WhileStmt();

    Index ExprStmt();

    //-- Parsing: Miscellaneous --//
    inline Index Vis();
    Index Type();
    Index Pattern();
    Index FnDef();
    Index Params();

    //-- Parsing: Expressions --//
    Index SingleOpExpr(SingleOp operation);
    Index ExprVal();
    //
    Index ExprPost(Index node, Operators& op);
    Index Expr();
    Index PathExpr();
    Index StructExpr();
    Index ClosureExpr();

    // Gets the operator. If the current token is not valid operator then returns Operators::INVALID.
    Operators GetOp();
    unsigned short GetOpPrecedence(Operators op);

    //-- Messages --//
    void ExpectSemicolon();
    void ErrUnexpected(std::string_view msg);

    //-- AST methods --//
    Index ReserveNode(Tag tag, Index token = NULL_INDEX);
    Index CreateNode(Tag tag, Index lhs, Index rhs, Index token = NULL_INDEX);

    void InsertMeta(Index node, const std::initializer_list<Index>& values);
    void InsertData(Index node, Index data);
    void SetNodeValues(Index node, Index lhs, Index rhs);

    void AddToCache(Index val);
    [[nodiscard]] Index CreateMetaFromCache(Index start);

    [[nodiscard]] Index GetCacheLen() const;
};

inline Index Parser::Vis() {
    return static_cast<Index>(isNull(m_ast.Tokens.EatTok(TokenTag::K_PUB)) ? Vis::LOCAL : Vis::GLOBAL);
}

}
#endif // KOOLANG_PARSER_H
