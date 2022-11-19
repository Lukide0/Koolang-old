#include "Parser.h"
#include "Node.h"
#include "Token.h"
#include "Tokenizer.h"
#include "codes/err.h"
#include "util/alias.h"
#include "util/debug.h"
#include <sstream>
#include <vector>

namespace ast {

using namespace logger;

Parser::Parser(File& file, std::string_view filepath)
    : m_filepath(filepath)
    , m_sourceCode(file.Content)
    , m_file(file) {
    m_ast.NodeTags.reserve(AST_BUFF_SIZE);
    m_ast.Nodes.reserve(AST_BUFF_SIZE);
    m_ast.NodeTokens.reserve(AST_BUFF_SIZE);
    m_ast.Meta.reserve(AST_BUFF_SIZE);

    // reserve 0 index
    m_ast.NodeTags.push_back(Tag::ROOT);
    m_ast.Nodes.emplace_back();
    m_ast.NodeTokens.push_back(NULL_INDEX);
    m_ast.Meta.push_back({});
}

Index Parser::ReserveNode(Tag tag, Index token) {
    m_ast.Nodes.emplace_back();
    m_ast.NodeTags.push_back(tag);
    m_ast.NodeTokens.push_back(token);

    return static_cast<Index>(m_ast.Nodes.size() - 1);
}

Index Parser::CreateNode(Tag tag, Index lhs, Index rhs, Index token) {
    m_ast.Nodes.emplace_back(lhs, rhs);
    m_ast.NodeTags.push_back(tag);
    m_ast.NodeTokens.push_back(token);

    return static_cast<Index>(m_ast.Nodes.size() - 1);
}

void Parser::SetNodeValues(Index node, Index lhs, Index rhs) {
    m_ast.Nodes[node].Lhs = lhs;
    m_ast.Nodes[node].Rhs = rhs;
}

void Parser::AddToCache(Index val) { m_cache.push_back(val); }

Index Parser::CreateMetaFromCache(Index start) {
    const auto meta = static_cast<Index>(m_ast.Meta.size());

    m_ast.Meta.insert(m_ast.Meta.end(), m_cache.begin() + start, m_cache.end());
    m_cache.resize(start);

    return meta;
}

Index Parser::GetCacheLen() const { return static_cast<Index>(m_cache.size()); }

void Parser::ErrUnexpected(std::string_view msg) {
    const auto currIndex = m_ast.Tokens.GetCurrIndex();
    const auto tokStart  = m_ast.Tokens.GetTokStart(currIndex);
    const auto tokEnd    = m_ast.Tokens.GetTokEnd(currIndex);

    m_file.AddMsg(Record(
        RecordKind::ERR, &m_file, codes::err::UNEXPECTED_TOKEN, "Unexpected symbol",
        Label(msg, Range { tokStart, tokEnd }).WithColor(Color::RED)
    ));
}

void Parser::InsertMeta(Index node, const std::initializer_list<Index>& values) {
    const auto start = static_cast<Index>(m_ast.Meta.size());

    m_ast.Meta.insert(m_ast.Meta.end(), values);
    m_ast.Nodes[node].Lhs = start;
}

void Parser::InsertData(Index node, Index data) { m_ast.Nodes[node].Rhs = data; }

Ast Parser::Parse() {
    {
        Tokenizer tokenizer(m_file);
        m_ast.Tokens = tokenizer.Tokenize();
    }

    Index result;
    auto tag = TokenTag::START_OF_FILE;
    while (m_ast.Tokens.GetCurrTok() != TokenTag::END_OF_FILE) {
        m_docTok = m_ast.Tokens.EatDocComments();
        m_vis    = Vis();

        tag = m_ast.Tokens.GetCurrTok();

        switch (tag) {
        case TokenTag::K_IMPORT:
            result = ImportStmt();
            break;
        case TokenTag::K_CONST:
            result = ConstantStmt();
            break;
        case TokenTag::K_FN:
            result = FnStmt();
            break;
        case TokenTag::K_STRUCT:
            result = StructStmt();
            break;
        case TokenTag::K_VARIANT:
            result = VariantStmt();
            break;
        case TokenTag::K_ENUM:
            result = EnumStmt();
            break;
        case TokenTag::K_TRAIT:
            result = TraitStmt();
            break;
        case TokenTag::K_IMPL:
            result = ImplStmt();
            break;
        case TokenTag::END_OF_FILE:
            result = NULL_INDEX;
            ErrUnexpected("Empty file");
            break;
        default:
            result = NULL_INDEX;
            ErrUnexpected("Expected global statement");
            break;
        }

        // fatal error
        if (isNull(result)) {
            break;
        }

        m_ast.Top.push_back(result);
    }

    return m_ast;
}

////////////////////////////////////////// MISCELLANEOUS //////////////////////////////////////////

/*
Syntax:
    path (*)* (&)?
    [ type ; expression ] (*)* &?
    ( type (, type)*  ) (*)* &?
    dyn path (+ path)* (*)* &?
    fn ( (type (, type)* )? ) (-> type)?
    |[ type ]|
*/
Index Parser::Type() {
    Index meta = NULL_INDEX;

    const Index node = ReserveNode(Tag::TYPE);
    Index base;

    switch (m_ast.Tokens.GetCurrTok()) {
    // path
    case TokenTag::IDENT:
        base = PathExpr();
        RETURN_IF_NULL(base);

        break;
    // [ type ; expression ]
    case TokenTag::SQUARE_L: {
        base             = ReserveNode(Tag::TYPE_ARR, m_ast.Tokens.EatTok());
        const Index type = Type();
        RETURN_IF_NULL(type);

        ExpectSemicolon();

        const Index expr = Expr();
        RETURN_IF_NULL(expr);

        if (!m_ast.Tokens.Expect(TokenTag::SQUARE_R)) {
            ErrUnexpected("Expected `]`");
            return NULL_INDEX;
        }

        SetNodeValues(base, type, expr);
        break;
    }
    // ( type (, type)+)
    case TokenTag::PAREN_L: {
        base = ReserveNode(Tag::TYPE_TUPLE, m_ast.Tokens.EatTok());

        const Index cacheIndex = GetCacheLen();

        Index size = 1;
        Index type = Type();
        RETURN_IF_NULL(type);

        AddToCache(type);

        do {
            if (!m_ast.Tokens.Expect(TokenTag::COMMA)) {
                ErrUnexpected("Expected `,`");
                return NULL_INDEX;
            }

            type = Type();
            size += 1;
            RETURN_IF_NULL(type);
            AddToCache(type);
        } while (!m_ast.Tokens.Expect(TokenTag::PAREN_R));

        SetNodeValues(base, CreateMetaFromCache(cacheIndex), size);
        break;
    }
    // dyn<path (+ path)>
    case TokenTag::K_DYN: {
        base = ReserveNode(Tag::TYPE_DYNAMIC, m_ast.Tokens.EatTok());

        Index size = 0;
        Index type;

        if (!m_ast.Tokens.Expect(TokenTag::LS)) {
            ErrUnexpected("Expected `<`");
            return NULL_INDEX;
        }

        const Index cacheIndex = GetCacheLen();
        do {
            type = PathExpr();
            size += 1;
            AddToCache(type);
            RETURN_IF_NULL(type);

        } while (m_ast.Tokens.Expect(TokenTag::ADD));

        if (!m_ast.Tokens.Expect(TokenTag::GT)) {
            ErrUnexpected("Expected `>`");
            return NULL_INDEX;
        }

        SetNodeValues(base, CreateMetaFromCache(cacheIndex), size);
        break;
    }
    // |[ type ]|
    case TokenTag::OR:
        // |
        m_ast.Tokens.SkipTok();

        // [
        if (!m_ast.Tokens.Expect(TokenTag::SQUARE_L)) {
            ErrUnexpected("Expected `[`");
            return NULL_INDEX;
        }

        base = Type();
        RETURN_IF_NULL(base);
        m_ast.NodeTags[base] = Tag::TYPE_SLICE;

        // ]
        if (!m_ast.Tokens.Expect(TokenTag::SQUARE_R)) {
            ErrUnexpected("Expected `]`");
            return NULL_INDEX;
        }

        // |
        if (!m_ast.Tokens.Expect(TokenTag::OR)) {
            ErrUnexpected("Expected `|`");
            return NULL_INDEX;
        }
        break;

    // fn ( (type (, type)* )? ) (-> type)?
    case TokenTag::K_FN: {
        base = ReserveNode(Tag::TYPE_FN, m_ast.Tokens.EatTok());

        if (!m_ast.Tokens.Expect(TokenTag::PAREN_L)) {
            ErrUnexpected("Expected `(`");
            return NULL_INDEX;
        }

        Index returnType = NULL_INDEX;
        Index metaType   = NULL_INDEX;
        Index size       = 0;

        if (!m_ast.Tokens.Expect(TokenTag::PAREN_R)) {
            const Index cacheIndex = GetCacheLen();
            // TMP size
            AddToCache(0);

            Index type;

            do {
                type = Type();

                RETURN_IF_NULL(type);

                size += 1;
                AddToCache(type);

            } while (m_ast.Tokens.Expect(TokenTag::COMMA));

            if (!m_ast.Tokens.Expect(TokenTag::PAREN_R)) {
                ErrUnexpected("Expected `)`");
                return NULL_INDEX;
            }

            m_cache[cacheIndex] = size;

            metaType = CreateMetaFromCache(cacheIndex);
        }

        if (m_ast.Tokens.Expect(TokenTag::ARROW)) {
            returnType = Type();
            RETURN_IF_NULL(returnType);
        }

        SetNodeValues(base, metaType, returnType);
        SetNodeValues(node, meta, base);
        return node;
    }

    default:
        ErrUnexpected("Expected type");
        return NULL_INDEX;
    }
    // (*)*
    while (m_ast.Tokens.Expect(TokenTag::STAR)) {
        meta++;
        if ((meta & TYPE_PTR_MASK) > POINTER_MAX) {
            KOOLANG_TODO();
            return NULL_INDEX;
        }
    }

    // &?
    if (m_ast.Tokens.Expect(TokenTag::AND)) {
        meta |= TYPE_FLAG_REFERENCE;
    }

    SetNodeValues(node, meta, base);

    return node;
}

/*
Syntax:
    _                                          // discard
    mut? ident : type                          // single
    (pattern (,pattern)* )                     // multiple
    path { ident -> ident (, ident -> ident) }   // struct
*/
Index Parser::Pattern() {
    switch (m_ast.Tokens.GetCurrTok()) {
    // _
    case TokenTag::UNDERSCORE:
        return CreateNode(Tag::PATTERN_DISCARD, NULL_INDEX, NULL_INDEX, m_ast.Tokens.EatTok());
    // (
    case TokenTag::PAREN_L: {
        const Index node = ReserveNode(Tag::PATTERN_MULTIPLE, m_ast.Tokens.EatTok());
        Index pattern;
        Index size             = 0;
        const Index cacheIndex = GetCacheLen();

        do {
            // pattern
            pattern = Pattern();
            size += 1;

            RETURN_IF_NULL(pattern);
            AddToCache(pattern);
            // (, pattern)*
        } while (m_ast.Tokens.Expect(TokenTag::COMMA));

        // )
        if (!m_ast.Tokens.Expect(TokenTag::PAREN_R)) {
            ErrUnexpected("Expected `)`");
            return NULL_INDEX;
        }

        SetNodeValues(node, CreateMetaFromCache(cacheIndex), size);
        return node;
    }
    // ident
    case TokenTag::K_MUT:
    case TokenTag::IDENT: {
        // struct pattern
        if (m_ast.Tokens.Peek(TokenTag::CURLY_L, 1) || m_ast.Tokens.Peek(TokenTag::COLON2, 1)) {
            break;
        }

        const auto isMutable = static_cast<Index>(m_ast.Tokens.Expect(TokenTag::K_MUT));
        const Index ident    = m_ast.Tokens.EatTok();
        const Index node     = ReserveNode(Tag::PATTERN_SINGLE, ident);
        Index type           = NULL_INDEX;

        // : type
        if (m_ast.Tokens.Expect(TokenTag::COLON)) {
            type = Type();
            RETURN_IF_NULL(type);
        }

        SetNodeValues(node, type, isMutable);
        return node;
    }
    default:
        ErrUnexpected("Expected pattern");
        return NULL_INDEX;
    }

    const Index node = ReserveNode(Tag::PATTERN_STRUCT, m_ast.Tokens.GetCurrIndex());

    const Index path = PathExpr();
    RETURN_IF_NULL(path);

    const Index blockNode  = ReserveNode(Tag::BLOCK, m_ast.Tokens.GetCurrIndex());
    const Index cacheIndex = GetCacheLen();

    if (!m_ast.Tokens.Expect(TokenTag::CURLY_L)) {
        ErrUnexpected("Expected `{`");
        return NULL_INDEX;
    }

    Index item;
    Index size = 0;

    do {
        const Index field = m_ast.Tokens.GetCurrIndex();

        if (!m_ast.Tokens.Expect(TokenTag::IDENT)) {
            ErrUnexpected("Expected field name");
            return NULL_INDEX;
        }

        if (!m_ast.Tokens.Expect(TokenTag::ARROW)) {
            ErrUnexpected("Expected `->`");
            return NULL_INDEX;
        }

        const auto isMutable = static_cast<Index>(m_ast.Tokens.Expect(TokenTag::K_MUT));

        const Index ident = m_ast.Tokens.GetCurrIndex();
        if (!m_ast.Tokens.Expect(TokenTag::IDENT)) {
            ErrUnexpected("Expected variable name");
            return NULL_INDEX;
        }

        item = CreateNode(Tag::PATTERN_STRUCT_FIELD, field, isMutable, ident);
        AddToCache(item);
        size += 1;

    } while (m_ast.Tokens.Expect(TokenTag::COMMA));

    if (!m_ast.Tokens.Expect(TokenTag::CURLY_R)) {
        ErrUnexpected("Expected `}`");
        return NULL_INDEX;
    }

    SetNodeValues(blockNode, CreateMetaFromCache(cacheIndex), size);
    SetNodeValues(node, path, blockNode);
    return node;
}
/*
Syntax:
    ( param*  )

    param:
        mut? ident : type
*/
Index Parser::Params() {
    const Index params     = ReserveNode(Tag::FN_PARAMS, m_ast.Tokens.EatTok());
    Index param            = NULL_INDEX;
    Index size             = 0;
    const Index cacheIndex = GetCacheLen();

    // )
    if (!m_ast.Tokens.Expect(TokenTag::PAREN_R)) {
        // param+
        do {
            bool isMut = m_ast.Tokens.Expect(TokenTag::K_MUT);
            // ident
            Index ident = m_ast.Tokens.EatTok(TokenTag::IDENT);
            if (isNull(ident)) {
                ErrUnexpected("Expected name of parameter");
                return NULL_INDEX;
            }
            param = ReserveNode(Tag::FN_PARAM, ident);

            // :
            if (!m_ast.Tokens.Expect(TokenTag::COLON)) {
                ErrUnexpected("Expected `:`");
                return NULL_INDEX;
            }
            // type
            Index type = Type();
            RETURN_IF_NULL(type);

            SetNodeValues(param, static_cast<Index>(isMut), type);
            size += 1;

            AddToCache(param);

        } while (m_ast.Tokens.Expect(TokenTag::COMMA));

        // )
        if (!m_ast.Tokens.Expect(TokenTag::PAREN_R)) {
            ErrUnexpected("Expected `)`");
            return NULL_INDEX;
        }
    }

    SetNodeValues(params, CreateMetaFromCache(cacheIndex), size);

    return params;
}

/*
Syntax:
    fn ident params const? (: type)?
*/
Index Parser::FnDef() {
    constexpr Index MOD_NONE  = NULL_INDEX;
    constexpr Index MOD_CONST = 1;

    // fn
    const Index node = ReserveNode(Tag::FN_DEF, m_ast.Tokens.EatTok());
    Index mods       = MOD_NONE;
    Index returnType = NULL_INDEX;

    // ident
    if (!m_ast.Tokens.Expect(TokenTag::IDENT)) {
        ErrUnexpected("Expected name of function");
        return NULL_INDEX;
    }

    // (
    if (!m_ast.Tokens.Peek(TokenTag::PAREN_L)) {
        ErrUnexpected("Expected `(`");
        return NULL_INDEX;
    }

    const Index params = Params();
    RETURN_IF_NULL(params);

    InsertData(node, params);

    // const?
    if (m_ast.Tokens.Expect(TokenTag::K_CONST)) {
        mods |= MOD_CONST;
    }

    // (: type)?
    if (m_ast.Tokens.Expect(TokenTag::COLON)) {
        returnType = Type();
        RETURN_IF_NULL(returnType);
    }

    InsertMeta(node, { returnType, mods, m_vis, m_docTok });
    return node;
}

void Parser::ExpectSemicolon() {
    if (isNull(m_ast.Tokens.EatTok(TokenTag::SEMI))) {
        const auto currIndex = m_ast.Tokens.GetCurrIndex() - 1;
        const auto tokEnd    = m_ast.Tokens.GetTokEnd(currIndex);

        m_file.AddMsg(Record(
            RecordKind::ERR, &m_file, codes::err::MISSING_SEMICOLON, "Missing semicolon",
            Label("", Range { tokEnd, tokEnd })
        ));
    }
}
}
