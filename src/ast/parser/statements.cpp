#include "Parser.h"
#include "util/debug.h"

namespace ast {

/*
Syntax:
    { statement* }

    statement:
        varStmt
        constantStmt
        exprStmt
        discardStmt
*/
Index Parser::BlockStmt() {
    const Index start = m_ast.Tokens.EatTok(TokenTag::CURLY_L);
    if (isNull(start)) {
        ErrUnexpected("Expected `{`");
        return NULL_INDEX;
    }

    const Index node       = ReserveNode(Tag::BLOCK, start);
    const Index cacheIndex = GetCacheLen();
    Index size             = 0;
    Index item             = NULL_INDEX;

    while (!m_ast.Tokens.Expect(TokenTag::CURLY_R)) {
        m_docTok = m_ast.Tokens.EatDocComments();

        switch (m_ast.Tokens.GetCurrTok()) {
        // _ = expression
        case TokenTag::UNDERSCORE: {
            item = ReserveNode(Tag::DISCARD, m_ast.Tokens.EatTok());

            if (!m_ast.Tokens.Expect(TokenTag::EQ)) {
                ErrUnexpected("Expected `=`");
                item = NULL_INDEX;
                break;
            }

            const Index expr = Expr();
            RETURN_IF_NULL(expr);

            InsertData(item, expr);
            ExpectSemicolon();
            break;
        }

        case TokenTag::K_VAR:
            item = VarStmt();
            break;
        case TokenTag::K_CONST:
            item = ConstantStmt();
            break;
        case TokenTag::K_BREAK:
            item = CreateNode(Tag::FLOW_OP, FLOW_BREAK, m_ast.Tokens.EatTok(TokenTag::IDENT), m_ast.Tokens.EatTok());
            ExpectSemicolon();
            break;
        case TokenTag::K_CONTINUE:
            item = CreateNode(Tag::FLOW_OP, FLOW_CONTINUE, m_ast.Tokens.EatTok(TokenTag::IDENT), m_ast.Tokens.EatTok());
            ExpectSemicolon();
            break;
        case TokenTag::K_RETURN: {
            item = ReserveNode(Tag::FLOW_OP, m_ast.Tokens.EatTok());

            if (m_ast.Tokens.Expect(TokenTag::SEMI)) {
                SetNodeValues(item, FLOW_RETURN, NULL_INDEX);
                break;
            }

            const Index expr = Expr();
            RETURN_IF_NULL(expr);

            SetNodeValues(item, 3, expr);

            ExpectSemicolon();

            break;
        }
        case TokenTag::K_IF:
            item = IfStmt();
            break;

        case TokenTag::K_FOR:
            item = ForStmt();
            break;

        case TokenTag::K_WHILE:
            item = WhileStmt();
            break;

        case TokenTag::K_STATIC:
            item = StaticStmt();
            break;

        default:
            item = ExprStmt();
            break;
        }

        RETURN_IF_NULL(item);
        AddToCache(item);

        size += 1;
    }

    SetNodeValues(node, CreateMetaFromCache(cacheIndex), size);
    return node;
}

/*
Syntax:
    import ident ( :: ident )* ( :: { ( ident ( :: ident )* (= ident)? , )+ })? ;

Examples:
    import a::b::c;
    import a::{
        b::c = c
        b2::d = d
    };
*/
Index Parser::ImportStmt() {
    // reserve node for import statement
    const Index node = ReserveNode(Tag::IMPORT, m_ast.Tokens.EatTok());
    m_ast.Imports.push_back(node);

    if (m_vis != NULL_INDEX) {
        InsertMeta(node, { m_vis });
    }

    const Index pathStart = m_ast.Tokens.EatTok(TokenTag::IDENT);
    if (isNull(pathStart)) {
        ErrUnexpected("Expected identifier");
        return NULL_INDEX;
    }

    Index importMultipleStart = NULL_INDEX;
    // base path
    while (m_ast.Tokens.Expect(TokenTag::COLON2)) {
        // check if the next token is identifier or {
        if (!m_ast.Tokens.Expect(TokenTag::IDENT)) {
            importMultipleStart = m_ast.Tokens.EatTok(TokenTag::CURLY_L);

            if (isNull(importMultipleStart)) {
                ErrUnexpected("Expected `{`");
                return NULL_INDEX;
            }

            break;
        }
    }

    Index pathEnd  = m_ast.Tokens.GetCurrIndex() - 1;
    Index aliasTok = NULL_INDEX;

    // insert base path
    Index importPath = ReserveNode(Tag::IMPORT_PATH, pathStart);

    // single import?
    if (isNull(importMultipleStart)) {
        // = ident
        if (m_ast.Tokens.Expect(TokenTag::EQ)) {
            aliasTok = m_ast.Tokens.EatTok(TokenTag::IDENT);
            if (isNull(aliasTok)) {
                ErrUnexpected("Expected identifier");
                return NULL_INDEX;
            }
        }

        SetNodeValues(importPath, CreateNode(Tag::PATH, pathStart, pathEnd, pathStart), aliasTok);
        InsertData(node, static_cast<Index>(m_ast.Nodes.size() - 1));

        ExpectSemicolon();

        return node;
    }

    SetNodeValues(importPath, CreateNode(Tag::PATH, pathStart, pathEnd - 2, pathStart), aliasTok);

    while (!m_ast.Tokens.Expect(TokenTag::CURLY_R)) {
        aliasTok = NULL_INDEX;
        // parse single IMPORT_PATH
        importPath = ReserveNode(Tag::IMPORT_PATH, m_ast.Tokens.GetCurrIndex());

        if (!m_ast.Tokens.Peek(TokenTag::IDENT)) {
            ErrUnexpected("Unexpected identifier");
            return NULL_INDEX;
        }
        Index path = PathExpr();

        // = ALIAS
        if (m_ast.Tokens.Expect(TokenTag::EQ)) {
            aliasTok = m_ast.Tokens.EatTok(TokenTag::IDENT);

            if (isNull(aliasTok)) {
                ErrUnexpected("Expected identifier");
                return NULL_INDEX;
            }
        }

        SetNodeValues(importPath, path, aliasTok);

        if (!m_ast.Tokens.Expect(TokenTag::COMMA) && m_ast.Tokens.Peek(TokenTag::IDENT)) {
            ErrUnexpected("Expected `,`");
        }
    }

    InsertData(node, static_cast<Index>(m_ast.Nodes.size() - 1));
    ExpectSemicolon();
    return node;
}

/*
Syntax:
    const ident : type = expr ;
Example:
    const WIDTH : u32 = 500;
*/
Index Parser::ConstantStmt() {
    // const
    const Index node = ReserveNode(Tag::CONSTANT, m_ast.Tokens.EatTok());
    // ident :
    if (!m_ast.Tokens.Expect(TokenTag::IDENT)) {
        ErrUnexpected("Expected constant name");
        return NULL_INDEX;
    }

    if (!m_ast.Tokens.Expect(TokenTag::COLON)) {
        ErrUnexpected("Expected `:`");
        return NULL_INDEX;
    }
    // type
    const Index type = Type();
    RETURN_IF_NULL(type);

    // =
    if (!m_ast.Tokens.Expect(TokenTag::EQ)) {
        ErrUnexpected("Expected `=`");
        return NULL_INDEX;
    }

    // expr
    const Index expr = Expr();
    RETURN_IF_NULL(expr);

    // ;
    ExpectSemicolon();

    InsertData(node, expr);
    InsertMeta(node, { type, m_vis, m_docTok });
    return node;
}

/*
Syntax:
    static ident (: type)? = expr ;
*/
Index Parser::StaticStmt() {
    // static
    const Index node = ReserveNode(Tag::STATIC, m_ast.Tokens.EatTok());
    // ident
    if (!m_ast.Tokens.Expect(TokenTag::IDENT)) {
        ErrUnexpected("Unexpected identifier");
        return NULL_INDEX;
    }

    Index type = NULL_INDEX;
    // type
    if (m_ast.Tokens.Expect(TokenTag::COLON)) {
        type = Type();
        RETURN_IF_NULL(type);
    }

    // =
    if (!m_ast.Tokens.Expect(TokenTag::EQ)) {
        ErrUnexpected("Expected `=`");
        return NULL_INDEX;
    }

    // expr
    const Index expr = Expr();
    RETURN_IF_NULL(expr);

    // ;
    ExpectSemicolon();

    InsertData(node, expr);
    InsertMeta(node, { type, m_vis, m_docTok });
    return node;
}
/*
Syntax:
    var pattern = expr;

Example:
    var x : i32 = 5;
    var y = 8;
    var (x : i32, y : u32) = (5, 6);
    var (x, y) = (5, 6);
*/
Index Parser::VarStmt() {
    // var
    const Index node = ReserveNode(Tag::VARIABLE, m_ast.Tokens.EatTok());

    // pattern
    const Index pattern = Pattern();
    RETURN_IF_NULL(pattern);

    InsertMeta(node, { pattern });

    Index expr = NULL_INDEX;

    // =
    if (!m_ast.Tokens.Expect(TokenTag::EQ)) {
        ErrUnexpected("Expected '='");
        return NULL_INDEX;
    }

    expr = Expr();
    RETURN_IF_NULL(expr);

    InsertData(node, expr);
    ExpectSemicolon();
    return node;
}

/*
Syntax:
    fnDef { statement* }
*/
Index Parser::FnStmt() {
    const Index node = ReserveNode(Tag::FN);

    // fnDef
    const Index fnDef = FnDef();
    RETURN_IF_NULL(fnDef);

    // { statement* }
    const Index body = BlockStmt();
    RETURN_IF_NULL(body);

    SetNodeValues(node, fnDef, body);
    return node;
}

/*
Syntax:
    variant ident { type (, type)* }
*/
Index Parser::VariantStmt() {
    // variant
    const Index node = ReserveNode(Tag::VARIANT, m_ast.Tokens.EatTok());

    // ident
    if (!m_ast.Tokens.Expect(TokenTag::IDENT)) {
        ErrUnexpected("Expected name of variant");
        return NULL_INDEX;
    }

    // {
    if (!m_ast.Tokens.Expect(TokenTag::CURLY_L)) {
        ErrUnexpected("Expected `{`");
        return NULL_INDEX;
    }

    const Index cacheIndex = GetCacheLen();
    AddToCache(m_vis);
    AddToCache(m_docTok);
    // type (, type)+
    Index field = 0;
    Index size  = 0;
    do {
        if (m_ast.Tokens.Peek(TokenTag::CURLY_R)) {
            break;
        }

        field = Type();
        RETURN_IF_NULL(field);
        size += 1;
        AddToCache(field);

    } while (m_ast.Tokens.Expect(TokenTag::COMMA));

    // }
    if (!m_ast.Tokens.Expect(TokenTag::CURLY_R)) {
        ErrUnexpected("Expected `}`");
        return NULL_INDEX;
    }

    SetNodeValues(node, CreateMetaFromCache(cacheIndex), size);
    return node;
}

/*
Syntax:
    enum ident (< type >)? { ident (= expr)? }
*/
Index Parser::EnumStmt() {
    // enum
    const Index node = ReserveNode(Tag::ENUM, m_ast.Tokens.EatTok());

    // ident
    if (!m_ast.Tokens.Expect(TokenTag::IDENT)) {
        ErrUnexpected("Expected name of the enum");
        return NULL_INDEX;
    }

    Index type = NULL_INDEX;
    // (< type >)?
    if (m_ast.Tokens.Expect(TokenTag::LS)) {
        type = Type();
        RETURN_IF_NULL(type);

        if (!m_ast.Tokens.Expect(TokenTag::GT)) {
            ErrUnexpected("Expected `>`");
            return NULL_INDEX;
        }
    }
    // {
    if (!m_ast.Tokens.Expect(TokenTag::CURLY_L)) {
        ErrUnexpected("Expected `{`");
        return NULL_INDEX;
    }

    Index field            = 0;
    Index fieldsCount      = 0;
    const Index cacheIndex = GetCacheLen();

    AddToCache(type);
    AddToCache(m_vis);
    AddToCache(m_docTok);

    do {
        // ident
        Index ident = m_ast.Tokens.EatTok(TokenTag::IDENT);
        if (isNull(ident)) {
            break;
        }

        field = ReserveNode(Tag::ENUM_FIELD, ident);

        Index expr = NULL_INDEX;
        if (isNull(ident)) {
            ErrUnexpected("Expected enum field name");
            return NULL_INDEX;
        }

        // (= expr)?
        if (m_ast.Tokens.Expect(TokenTag::EQ)) {
            expr = Expr();
            RETURN_IF_NULL(expr);
        }

        SetNodeValues(field, ident, expr);
        fieldsCount += 1;

        AddToCache(field);
    } while (m_ast.Tokens.Expect(TokenTag::COMMA));

    // }
    if (!m_ast.Tokens.Expect(TokenTag::CURLY_R)) {
        ErrUnexpected("Expected `}`");
        return NULL_INDEX;
    }
    SetNodeValues(node, CreateMetaFromCache(cacheIndex), fieldsCount);
    return node;
}

/*
Syntax:
    struct ident
    {
        (structItem ;)+
    }

    structItem:
        const ident : type = expr
        pub? ident : type (= expr)?

*/
Index Parser::StructStmt() {
    // struct
    const Index node = ReserveNode(Tag::STRUCT, m_ast.Tokens.EatTok());
    Index field;

    // ident
    Index ident = m_ast.Tokens.EatTok(TokenTag::IDENT);
    if (isNull(ident)) {
        ErrUnexpected("Expected name of a struct");
        return NULL_INDEX;
    }

    if (!m_ast.Tokens.Expect(TokenTag::CURLY_L)) {
        ErrUnexpected("Expected `{`");
        return NULL_INDEX;
    }

    const Index cacheIndex = GetCacheLen();
    Index fieldsCount      = 0;

    AddToCache(m_vis);
    AddToCache(m_docTok);

    do {
        m_docTok = m_ast.Tokens.EatDocComments();
        m_vis    = Vis();

        if (m_ast.Tokens.Expect(TokenTag::K_CONST)) {

            const Index fieldIdent = m_ast.Tokens.EatTok(TokenTag::IDENT);
            // const
            field = ReserveNode(Tag::STRUCT_CONST, fieldIdent);

            if (isNull(fieldIdent)) {
                ErrUnexpected("Expected name of a constant");
                return NULL_INDEX;
            }

            // :
            if (!m_ast.Tokens.Expect(TokenTag::COLON)) {
                ErrUnexpected("Expected `:`");
                return NULL_INDEX;
            }

            Index type = Type();
            RETURN_IF_NULL(type);

            // =
            if (!m_ast.Tokens.Expect(TokenTag::EQ)) {
                ErrUnexpected("Expected `=`");
                return NULL_INDEX;
            }

            Index expr = Expr();
            RETURN_IF_NULL(expr);

            InsertData(field, expr);
            InsertMeta(field, { type, m_vis, m_docTok });
        }
        // struct field
        else if (m_ast.Tokens.Peek(TokenTag::IDENT)) {
            // ident
            field = ReserveNode(Tag::STRUCT_FIELD, m_ast.Tokens.EatTok());

            // :
            if (!m_ast.Tokens.Expect(TokenTag::COLON)) {
                ErrUnexpected("Expected `:`");
                return NULL_INDEX;
            }

            const Index type = Type();
            RETURN_IF_NULL(type);
            Index expr = NULL_INDEX;

            // (= expr)?
            if (m_ast.Tokens.Expect(TokenTag::EQ)) {
                expr = Expr();
                RETURN_IF_NULL(expr);
            }
            InsertData(field, expr);
            InsertMeta(field, { type, m_vis, m_docTok });
        } else {
            ErrUnexpected("Expected `const` or identifier");
            return NULL_INDEX;
        }

        AddToCache(field);
        fieldsCount += 1;
        ExpectSemicolon();

    } while (!m_ast.Tokens.Expect(TokenTag::CURLY_R));

    SetNodeValues(node, CreateMetaFromCache(cacheIndex), fieldsCount);

    return node;
}

/*
Syntax:
    if ( expression ) blockStmt (else ifStmt|blockStmt )?
    else
    {}
*/
Index Parser::IfStmt() {
    const Index node = ReserveNode(Tag::IF_STMT, m_ast.Tokens.EatTok());

    if (!m_ast.Tokens.Expect(TokenTag::PAREN_L)) {
        ErrUnexpected("Expected `(`");
        return NULL_INDEX;
    }

    const Index cond = Expr();
    RETURN_IF_NULL(cond);

    if (!m_ast.Tokens.Expect(TokenTag::PAREN_R)) {
        ErrUnexpected("Expected `)`");
        return NULL_INDEX;
    }

    const Index block = BlockStmt();
    RETURN_IF_NULL(block);

    Index next = NULL_INDEX;

    if (m_ast.Tokens.Expect(TokenTag::K_ELSE)) {
        if (m_ast.Tokens.Peek(TokenTag::K_IF)) {
            next = IfStmt();
        } else {
            next = BlockStmt();
        }

        RETURN_IF_NULL(next);
    }

    InsertData(node, block);
    InsertMeta(node, { cond, next });

    return node;
}

/*
Syntax:
    trait ident { (fnDef ;)+ }
*/
Index Parser::TraitStmt() {
    // trait
    const Index node = ReserveNode(Tag::TRAIT, m_ast.Tokens.EatTok());
    Index endNode;

    // ident
    const Index ident = m_ast.Tokens.EatTok(TokenTag::IDENT);
    if (isNull(ident)) {
        ErrUnexpected("Expected name of a trait");
        return NULL_INDEX;
    }

    // {
    if (!m_ast.Tokens.Expect(TokenTag::CURLY_L)) {
        ErrUnexpected("Expected `{`");
        return NULL_INDEX;
    }

    // fnDef+ }
    do {
        endNode = FnDef();
        RETURN_IF_NULL(endNode);
        ExpectSemicolon();

    } while (!m_ast.Tokens.Expect(TokenTag::CURLY_R));

    InsertData(node, endNode);
    InsertMeta(node, { m_vis, m_docTok });
    return node;
}

/*
Syntax:
    impl pathExpr (: pathExpr)? { fnStmt+ }
*/
Index Parser::ImplStmt() {
    // impl
    const Index node = ReserveNode(Tag::IMPL, m_ast.Tokens.EatTok());

    // pathExpr
    const Index path = PathExpr();
    Index trait      = NULL_INDEX;
    RETURN_IF_NULL(path);

    // (: pathExpr)?
    if (m_ast.Tokens.Expect(TokenTag::COLON)) {
        trait = PathExpr();
        RETURN_IF_NULL(trait);
    }

    // {
    if (!m_ast.Tokens.Expect(TokenTag::CURLY_L)) {
        ErrUnexpected("Expected `{`");
        return NULL_INDEX;
    }

    InsertMeta(node, { path, trait });

    const Index body = ReserveNode(Tag::IMPL_BODY);
    Index size       = 0;
    Index endNode;

    m_vis = static_cast<Index>(Vis::LOCAL);
    // fnStmt+ }
    do {

        m_docTok = m_ast.Tokens.EatDocComments();

        endNode = FnStmt();
        RETURN_IF_NULL(endNode);
        size += 1;

    } while (!m_ast.Tokens.Expect(TokenTag::CURLY_R));

    SetNodeValues(body, size, endNode);

    InsertData(node, body);
    return node;
}

/*
Syntax:
    expression ;

Example:
    x = 5;
    y.val += x;
*/
Index Parser::ExprStmt() {
    // expr
    const Index node = Expr();
    RETURN_IF_NULL(node);
    // ;
    ExpectSemicolon();
    return node;
}

/*
Syntax:
    for pattern in expression blockStmt

*/
Index Parser::ForStmt() {
    const Index node    = ReserveNode(Tag::FOR_STMT, m_ast.Tokens.EatTok());
    const Index pattern = Pattern();

    Index label = NULL_INDEX;
    RETURN_IF_NULL(pattern);

    if (!m_ast.Tokens.Expect(TokenTag::K_IN)) {
        ErrUnexpected("Expected `in`");
        return NULL_INDEX;
    }

    const Index expr = Expr();
    RETURN_IF_NULL(expr);

    if (m_ast.Tokens.Expect(TokenTag::COLON)) {
        label = m_ast.Tokens.EatTok(TokenTag::IDENT);
        if (isNull(label)) {
            ErrUnexpected("Expected label name");
            return NULL_INDEX;
        }
    }

    const Index block = BlockStmt();
    RETURN_IF_NULL(block);

    InsertData(node, block);
    InsertMeta(node, { pattern, expr, label });

    return node;
}

/*
Syntax:
    while expression  blockStmt
*/
Index Parser::WhileStmt() {
    const Index node = ReserveNode(Tag::WHILE_STMT, m_ast.Tokens.EatTok());
    const Index expr = Expr();
    Index label      = NULL_INDEX;
    RETURN_IF_NULL(expr);

    if (m_ast.Tokens.Expect(TokenTag::COLON)) {
        label = m_ast.Tokens.EatTok(TokenTag::IDENT);
        if (isNull(label)) {
            ErrUnexpected("Expected label name");
            return NULL_INDEX;
        }
    }

    const Index block = BlockStmt();
    RETURN_IF_NULL(block);

    InsertData(node, block);
    InsertMeta(node, { expr, label });

    return node;
}

}
