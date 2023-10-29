#include "ast/Parser.h"
#include "codes/err.h"

namespace ast {

bool IsExprPost(Operators operation) { return operation == Operators::ACCESS_ARR || operation == ast::Operators::CALL; }

Operators Parser::GetOp() {

    using Op = Operators;

    Op operation;
    switch (m_ast.Tokens.GetCurrTok()) {
    // call
    case TokenTag::PAREN_L:
        operation = Op::CALL;
        break;
    // access instance
    case TokenTag::DOT:
        operation = Op::ACCESS;
        break;
    // access pointer
    case TokenTag::ARROW:
        operation = Op::ACCESS_PTR;
        break;
    // access array
    case TokenTag::SQUARE_L:
        operation = Op::ACCESS_ARR;
        break;

    // unwrap
    case TokenTag::QUESTION:
        operation = Op::UNWRAP;
        break;
    case TokenTag::STAR:
        operation = Op::MUL;
        break;
    case TokenTag::MOD:
        operation = Op::MOD;
        break;
    case TokenTag::DIV:
        operation = Op::DIV;
        break;
    case TokenTag::ADD:
        operation = Op::ADD;
        break;
    case TokenTag::MINUS:
        operation = Op::SUB;
        break;
    case TokenTag::LS: {
        if (m_ast.Tokens.Peek(TokenTag::LS, 1)) {
            m_ast.Tokens.SkipTok();
            operation = Op::SHIFT_L;
        } else if (m_ast.Tokens.Peek(TokenTag::EQ, 1)) {
            m_ast.Tokens.SkipTok();
            operation = Op::LS_EQ;
        } else {
            operation = Op::LS;
        }
        break;
    }
    case TokenTag::GT: {
        if (m_ast.Tokens.Peek(TokenTag::GT, 1)) {
            m_ast.Tokens.SkipTok();
            operation = Op::SHIFT_R;
        } else if (m_ast.Tokens.Peek(TokenTag::EQ, 1)) {
            m_ast.Tokens.SkipTok();
            operation = Op::GT_EQ;
        } else {
            operation = Op::GT;
        }
        break;
    }
    case TokenTag::NOT_EQ:
        operation = Op::NOT_EQ;
        break;
    case TokenTag::AND:
        operation = Op::AND;
        break;
    case TokenTag::CARET:
        operation = Op::XOR;
        break;
    case TokenTag::OR:
        operation = Op::OR;
        break;
    case TokenTag::OR_OR:
        operation = Op::OR_OR;
        break;
    case TokenTag::AND_AND:
        operation = Op::AND_AND;
        break;
    case TokenTag::EQ_EQ:
        operation = Op::EQ_EQ;
        break;
    case TokenTag::EQ:
        operation = Op::EQ;
        break;
    case TokenTag::ADD_EQ:
        operation = Op::EQ_ADD;
        break;
    case TokenTag::MINUS_EQ:
        operation = Op::EQ_SUB;
        break;
    case TokenTag::STAR_EQ:
        operation = Op::EQ_MUL;
        break;
    case TokenTag::DIV_EQ:
        operation = Op::EQ_DIV;
        break;
    case TokenTag::MOD_EQ:
        operation = Op::EQ_MOD;
        break;
    case TokenTag::AND_EQ:
        operation = Op::EQ_AND;
        break;
    case TokenTag::OR_EQ:
        operation = Op::EQ_OR;
        break;
    case TokenTag::CARET_EQ:
        operation = Op::EQ_XOR;
        break;
    default:
        return Op::INVALID;
    }

    m_ast.Tokens.SkipTok();
    return operation;
}

unsigned short Parser::GetOpPrecedence(Operators op) {
    switch (op) {
    case Operators::INVALID:
        return 0;
    case Operators::ACCESS:
    case Operators::ACCESS_PTR:
    case Operators::ACCESS_ARR:
    case Operators::UNWRAP:
        return 12;
    case Operators::CALL:
        return 11;
    case Operators::MUL:
    case Operators::MOD:
    case Operators::DIV:
        return 10;
    case Operators::ADD:
    case Operators::SUB:
        return 9;
    case Operators::SHIFT_L:
    case Operators::SHIFT_R:
        return 8;
    case Operators::LS:
    case Operators::GT:
    case Operators::LS_EQ:
    case Operators::GT_EQ:
        return 7;
    case Operators::EQ_EQ:
    case Operators::NOT_EQ:
        return 6;
    case Operators::AND:
        return 5;
    case Operators::XOR:
        return 4;
    case Operators::OR:
        return 3;
    case Operators::OR_OR:
    case Operators::AND_AND:
        return 2;
    case Operators::EQ:
    case Operators::EQ_ADD:
    case Operators::EQ_SUB:
    case Operators::EQ_DIV:
    case Operators::EQ_MUL:
    case Operators::EQ_MOD:
    case Operators::EQ_OR:
    case Operators::EQ_AND:
    case Operators::EQ_XOR:
        return 1;
    }

    return 0;
}

Index Parser::SingleOpExpr(SingleOp operation) {
    using namespace logger;

    const Index node = ReserveNode(Tag::SINGLE_OP, m_ast.Tokens.EatTok());
    const Index val  = ExprVal();

    RETURN_IF_NULL(val);

    if (m_ast.NodeTags.at(val) == Tag::SINGLE_OP) {
        const Index tok     = m_ast.NodeTokens.at(val);
        const auto tokStart = m_ast.Tokens.GetTokStart(tok);
        const auto tokEnd   = m_ast.Tokens.GetTokEnd(tok);

        m_file.AddMsg(Record(
            RecordKind::ERR, &m_file, codes::err::MULTIPLE_UNARY_OPS, "Multiple unary operations",
            Label("", Range { tokStart, tokEnd })
        ));

        return NULL_INDEX;
    }

    SetNodeValues(node, static_cast<Index>(operation), val);
    return node;
}

Index Parser::ExprVal() {
    m_ast.Tokens.EatDocComments();

    Index node = NULL_INDEX;
    switch (m_ast.Tokens.GetCurrTok()) {
    // struct constructor
    case TokenTag::K_NEW:
        node = StructExpr();
        break;
    // tuple or grouped expression
    // (expr, expr) or ( expr )
    case TokenTag::PAREN_L: {
        node       = ReserveNode(Tag::TUPLE, m_ast.Tokens.EatTok());
        Index size = 1;
        Index expr = Expr();

        RETURN_IF_NULL(expr);

        if (!m_ast.Tokens.Expect(TokenTag::COMMA)) {
            m_ast.NodeTags[node] = Tag::GROUPED_EXPR;
            InsertData(node, expr);

            if (!m_ast.Tokens.Expect(TokenTag::PAREN_R)) {
                ErrUnexpected("Expected `)`");
                return NULL_INDEX;
            }

            break;
        }

        const Index cacheIndex = GetCacheLen();
        AddToCache(expr);

        while (!m_ast.Tokens.Expect(TokenTag::PAREN_R)) {
            expr = Expr();
            size += 1;

            RETURN_IF_NULL(expr);
            AddToCache(expr);

            if (m_ast.Tokens.Expect(TokenTag::COMMA) && m_ast.Tokens.Peek(TokenTag::PAREN_R)) {
                ErrUnexpected("Expected expression");
                break;
            }
        }

        SetNodeValues(node, CreateMetaFromCache(cacheIndex), size);
        break;
    }
    // array
    case TokenTag::SQUARE_L: {
        node = ReserveNode(Tag::ARRAY, m_ast.Tokens.EatTok());

        Index expr = Expr();
        Index size = 1;

        RETURN_IF_NULL(expr);

        // [value;size]
        if (m_ast.Tokens.Expect(TokenTag::SEMI)) {
            size = Expr();
            RETURN_IF_NULL(size);

            m_ast.NodeTags[node] = Tag::ARRAY_SHORT;
            SetNodeValues(node, size, expr);
        }
        // [value, value, ...]
        else {
            const Index cacheIndex = GetCacheLen();
            AddToCache(expr);

            while (m_ast.Tokens.Expect(TokenTag::COMMA)) {
                expr = Expr();
                size += 1;
                RETURN_IF_NULL(expr);
                AddToCache(expr);
            }

            SetNodeValues(node, CreateMetaFromCache(cacheIndex), size);
        }

        if (!m_ast.Tokens.Expect(TokenTag::SQUARE_R)) {
            ErrUnexpected("Expected `]`");
            return NULL_INDEX;
        }

        break;
    }

    // TODO: Check string and char literals for unknown escape sequences

    // literals
    case TokenTag::STRING_LIT:
        node = ReserveNode(Tag::LITERAL, m_ast.Tokens.GetCurrIndex());
        SetNodeValues(node, LITERAL_STRING, m_ast.Tokens.EatTok());
        break;
    case TokenTag::CHAR_LIT:
        node = ReserveNode(Tag::LITERAL, m_ast.Tokens.GetCurrIndex());
        SetNodeValues(node, LITERAL_CHAR, m_ast.Tokens.EatTok());
        break;
    case TokenTag::NUMBER_LIT:
        node = ReserveNode(Tag::LITERAL, m_ast.Tokens.GetCurrIndex());
        SetNodeValues(node, LITERAL_NUMBER, m_ast.Tokens.EatTok());
        break;
    case TokenTag::FLOAT_LIT:
        node = ReserveNode(Tag::LITERAL, m_ast.Tokens.GetCurrIndex());
        SetNodeValues(node, LITERAL_FLOAT, m_ast.Tokens.EatTok());
        break;

    // paths
    case TokenTag::IDENT:
        node = PathExpr();
        break;

    // bool invert
    case TokenTag::BANG:
        node = SingleOpExpr(SingleOp::BOOL_NEG);
        break;
    // bin invert
    case TokenTag::TILDE:
        node = SingleOpExpr(SingleOp::BIT_NEG);
        break;
    // ptr
    case TokenTag::AND:
        node = SingleOpExpr(SingleOp::GET_ADDR);
        break;
    // number invert
    case TokenTag::MINUS:
        node = SingleOpExpr(SingleOp::INT_NEG);
        break;
    // derefence
    case TokenTag::STAR:
        node = SingleOpExpr(SingleOp::DEREF);
        break;
    // slice
    case TokenTag::OR: {
        // |[base; from; to]|
        node = ReserveNode(Tag::SLICE_OP, m_ast.Tokens.EatTok());

        if (!m_ast.Tokens.Expect(TokenTag::SQUARE_L)) {
            ErrUnexpected("Expected `[`");
            return NULL_INDEX;
        }
        Index base = Expr();
        RETURN_IF_NULL(base);

        if (!m_ast.Tokens.Expect(TokenTag::SEMI)) {
            ErrUnexpected("Expected `;`");
            return NULL_INDEX;
        }

        Index from = NULL_INDEX;
        Index to   = NULL_INDEX;

        if (!m_ast.Tokens.Expect(TokenTag::UNDERSCORE)) {
            from = Expr();
            RETURN_IF_NULL(from);
        }

        Index comma = m_ast.Tokens.EatTok(TokenTag::COMMA);
        // check if the range is valid, e.g. |[base;_]| is not valid
        if (isNull(from) && isNull(comma)) {
            ErrUnexpected("Expected valid range");
            return NULL_INDEX;
        } else if (!isNull(comma)) {
            to = Expr();
            RETURN_IF_NULL(to);
        }

        if (!m_ast.Tokens.Expect(TokenTag::SQUARE_R)) {
            ErrUnexpected("Expected `]`");
            return NULL_INDEX;
        }

        if (!m_ast.Tokens.Expect(TokenTag::OR)) {
            ErrUnexpected("Expected `|`");
            return NULL_INDEX;
        }

        InsertData(node, base);
        InsertMeta(node, { from, to });

        break;
    }

    // cast < type > ( expr )
    case TokenTag::K_CAST: {
        node = ReserveNode(Tag::CAST_EXPR, m_ast.Tokens.EatTok());

        if (!m_ast.Tokens.Expect(TokenTag::LS)) {
            ErrUnexpected("Expected `<`");
            return NULL_INDEX;
        }

        const Index type = Type();
        RETURN_IF_NULL(type);

        if (!m_ast.Tokens.Expect(TokenTag::GT)) {
            ErrUnexpected("Expected `>`");
            return NULL_INDEX;
        }

        if (!m_ast.Tokens.Expect(TokenTag::PAREN_L)) {
            ErrUnexpected("Expected `(`");
            return NULL_INDEX;
        }

        const Index expr = Expr();
        RETURN_IF_NULL(expr);

        if (!m_ast.Tokens.Expect(TokenTag::PAREN_R)) {
            ErrUnexpected("Expected `)`");
            return NULL_INDEX;
        }

        SetNodeValues(node, type, expr);
        break;
    }
    default:
        ErrUnexpected("Expected expression");
        return NULL_INDEX;
    }

    return node;
}

Index Parser::ExprPost(Index node, Operators& op) {
    while (IsExprPost(op)) {
        switch (op) {
        case Operators::ACCESS_ARR: {
            const Index expr = Expr();
            RETURN_IF_NULL(expr);

            node = CreateNode(Tag::BIN_OP, node, expr, static_cast<Index>(ast::Operators::ACCESS_ARR));

            if (!m_ast.Tokens.Expect(TokenTag::SQUARE_R)) {
                ErrUnexpected("Expected ']'");
            }

            break;
        }
        case Operators::CALL: {
            // 0 args
            if (m_ast.Tokens.Expect(TokenTag::PAREN_R)) {
                node = CreateNode(Tag::CALL_OP, NULL_INDEX, node);
            }
            // 1+ args
            else {
                const Index callNode = ReserveNode(Tag::CALL_OP);

                const Index cacheIndex = GetCacheLen();

                AddToCache(0);

                Index size = 0;
                Index expr = NULL_INDEX;

                do {
                    expr = Expr();
                    RETURN_IF_NULL(expr);
                    size += 1;
                    AddToCache(expr);

                } while (m_ast.Tokens.Expect(TokenTag::COMMA));

                if (!m_ast.Tokens.Expect(TokenTag::PAREN_R)) {
                    ErrUnexpected("Expected `)`");
                    return NULL_INDEX;
                }

                m_cache[cacheIndex] = size;

                SetNodeValues(callNode, CreateMetaFromCache(cacheIndex), node);

                node = callNode;
            }
            break;
        }
        default:
            break;
        }

        op = GetOp();
    }

    return node;
}

Index Parser::Expr() {
    switch (m_ast.Tokens.GetCurrTok()) {
    // struct constructor
    case TokenTag::K_NEW:
        return StructExpr();
    // closure
    case TokenTag::K_FN:
        return ClosureExpr();
    default:
        break;
    }

    Index node = ExprVal();
    RETURN_IF_NULL(node);

    Operators op = GetOp();
    if (op == Operators::INVALID) {
        return node;
    } else if (op == Operators::UNWRAP) {
        node = CreateNode(Tag::UNWRAP_OP, NULL_INDEX, node);
        op   = GetOp();
        if (op == Operators::INVALID) {
            return node;
        }
    }

    node = ExprPost(node, op);
    RETURN_IF_NULL(node);

    std::vector<Index> values  = { node };
    std::vector<Operators> ops = { op };

    unsigned short precedence = GetOpPrecedence(op);

    while (op != Operators::INVALID) {
        // add rhs
        Index rhs = ExprVal();
        RETURN_IF_NULL(rhs);

        op = GetOp();

        // NOTE: Expression can look like rhs()()
        rhs = ExprPost(rhs, op);
        RETURN_IF_NULL(rhs);

        unsigned short currPrecedence = GetOpPrecedence(op);

        // the current operation has higher priority
        if (currPrecedence > precedence) {
            ops.push_back(op);
            values.push_back(rhs);
            precedence = currPrecedence;
            continue;
        }

        unsigned short tmp = precedence;
        // the current operation has lower or equal priority
        while (tmp >= currPrecedence) {
            rhs = CreateNode(Tag::BIN_OP, values.back(), rhs, static_cast<Index>(ops.back()));
            values.pop_back();
            ops.pop_back();

            if (ops.empty()) {
                break;
            }

            tmp = GetOpPrecedence(ops.back());
        }

        if (op == Operators::UNWRAP) {
            rhs = CreateNode(Tag::UNWRAP_OP, NULL_INDEX, rhs);
            op  = GetOp();
        }

        if (op == Operators::INVALID) {
            node = rhs;
            break;
        }

        values.push_back(rhs);
        ops.push_back(op);
        precedence = currPrecedence;
    }

    return node;
}

/*
Syntax:
    ident (:: ident)*
*/
Index Parser::PathExpr() {
    // ident
    const Index start = m_ast.Tokens.EatTok(TokenTag::IDENT);

    // ::
    while (m_ast.Tokens.Expect(TokenTag::COLON2)) {
        // ident
        if (!m_ast.Tokens.Expect(TokenTag::IDENT)) {
            ErrUnexpected("Expected identifier");
            return NULL_INDEX;
        }
    }

    const Index end = m_ast.Tokens.GetCurrIndex() - 1;
    return CreateNode(Tag::PATH, start, end, end);
}

/*
Syntax:
    new pathExpr ({ ident = expr (,ident = expr)* })?
*/
Index Parser::StructExpr() {
    const Index node = ReserveNode(Tag::STRUCT_EXPR, m_ast.Tokens.EatTok());

    const Index path = PathExpr();
    RETURN_IF_NULL(path);

    if (!m_ast.Tokens.Expect(TokenTag::CURLY_L)) {
        SetNodeValues(node, path, NULL_INDEX);
        return node;
    }

    const Index fields = ReserveNode(Tag::STRUCT_EXPR_FIELDS);
    Index field;
    Index size = 0;

    const Index cacheIndex = GetCacheLen();

    do {
        const Index ident = m_ast.Tokens.EatTok(TokenTag::IDENT);
        if (isNull(ident)) {
            ErrUnexpected("Expected `ident = expression`");
            return NULL_INDEX;
        }

        field = ReserveNode(Tag::STRUCT_EXPR_FIELD, ident);
        size += 1;

        if (!m_ast.Tokens.Expect(TokenTag::EQ)) {
            ErrUnexpected("Expected `=`");
            return NULL_INDEX;
        }

        const Index expr = Expr();
        RETURN_IF_NULL(expr);

        SetNodeValues(field, ident, expr);
        AddToCache(field);

    } while (m_ast.Tokens.Expect(TokenTag::COMMA));

    if (!m_ast.Tokens.Expect(TokenTag::CURLY_R)) {
        ErrUnexpected("Expected `}`");
        return NULL_INDEX;
    }

    SetNodeValues(fields, CreateMetaFromCache(cacheIndex), size);
    SetNodeValues(node, path, fields);
    return node;
}

/*
Syntax:
    fn[(mut? &? expr,)* ] params (: type)? blockstatement
}
*/
Index Parser::ClosureExpr() {
    constexpr Index FLAG_MUTABLE   = 1;
    constexpr Index FLAG_REFERENCE = 2;

    const Index node = ReserveNode(Tag::CLOSURE_EXPR, m_ast.Tokens.EatTok());

    if (!m_ast.Tokens.Expect(TokenTag::SQUARE_L)) {
        ErrUnexpected("Expected `[`");
        return NULL_INDEX;
    }

    const Index captures = ReserveNode(Tag::CLOSURE_CAPTURES);

    Index size    = 0;
    Index endNode = NULL_INDEX;

    if (!m_ast.Tokens.Expect(TokenTag::SQUARE_R)) {
        do {
            Index flags = 0;
            if (m_ast.Tokens.Expect(TokenTag::K_MUT)) {
                flags |= FLAG_MUTABLE;
            }

            if (m_ast.Tokens.Expect(TokenTag::AND)) {
                flags |= FLAG_REFERENCE;
            }

            const Index value = Expr();
            RETURN_IF_NULL(value);

            endNode = CreateNode(Tag::CLOSURE_CAPTURE, flags, value);
            size += 1;

        } while (m_ast.Tokens.Expect(TokenTag::COMMA));

        if (!m_ast.Tokens.Expect(TokenTag::SQUARE_R)) {
            ErrUnexpected("Expected `]`");
            return NULL_INDEX;
        }
    }

    SetNodeValues(captures, size, endNode);

    if (!m_ast.Tokens.Peek(TokenTag::PAREN_L)) {
        ErrUnexpected("Expected `(`");
        return NULL_INDEX;
    }

    const Index params = Params();
    RETURN_IF_NULL(params);

    Index returnType = NULL_INDEX;

    if (m_ast.Tokens.Expect(TokenTag::COLON)) {
        returnType = Type();
        RETURN_IF_NULL(returnType);
    }

    const Index block = BlockStmt();
    RETURN_IF_NULL(block);

    InsertData(node, block);
    InsertMeta(node, { captures, params, returnType });

    return node;
}

}
