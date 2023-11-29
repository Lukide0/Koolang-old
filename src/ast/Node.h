#ifndef KOOLANG_ASTNODE_H
#define KOOLANG_ASTNODE_H

#include "util/Index.h"
#include "util/alias.h"

namespace ast {

struct Node {
    Index Lhs;
    Index Rhs;

    Node(Index lhs = NULL_INDEX, Index rhs = NULL_INDEX)
        : Lhs(lhs)
        , Rhs(rhs) { }
};

ASSERT_8BYTES(Node);

// TODO: Buildin functions -> @sizeof, @typeof, ...
enum class Tag : unsigned char {

    /*
    Lhs: None
    Rhs: None
    */
    ROOT,

    /*
    Lhs: Meta
    Rhs: Size
    Meta: trailing nodes
    */
    BLOCK,

    /*
    Lhs: vis
    Rhs: End node

    */
    IMPORT,

    /*
    Lhs: PATH
    Rhs: IDENT token (alias)
    */
    IMPORT_PATH,

    /*
    Lhs: First IDENT token
    Rhs: Last IDENT token
    */
    PATH,

    /*
    Lhs: Meta
    Rhs: expression
    Meta: type, vis, doc token
    */
    CONSTANT,

    /*
    Lhs: Meta
    Rhs: expression
    Meta: pattern
    */
    VARIABLE,

    /*
    Lhs: Meta
    Rhs: expression
    Meta: type, vis, doc token
    */
    STATIC,

    /*
    Lhs: FN_DEF
    Rhs: BLOCK
    */
    FN,

    /*
    Lhs: Meta
    Rhs: FN_PARAMS
    Meta: type, modifiers, vis, doc tok
    Modifiers:
        0 -> None
        1 -> const
    */
    FN_DEF,

    /*
    Lhs: Meta
    Rhs: Size
    Meta: trailing FN_PARAM
    */
    FN_PARAMS,

    /*
    Lhs: Is mutable
    Rhs: Type
    */
    FN_PARAM,

    /*
    Lhs: Meta
    Rhs: Fields count
    Meta: vis, doc tok, trailing fields
    */
    VARIANT,

    /*
    Lhs: Meta
    Rhs: Size
    Meta: vis, PATH (struct), PATH (trait), trailing nodes
    */
    IMPL,

    /*
    Lhs: Meta
    Rhs: fields count
    Meta: vis, doc tok, trailing STRUCT_FIELD and STRUCT_CONST
    */
    STRUCT,

    /*
    Lhs: Meta
    Rhs: expression
    Meta: type, vis, doc tok
    */
    STRUCT_FIELD,

    /*
    Lhs: Meta
    Rhs: expression
    Meta: type, vis, doc tok
    */
    STRUCT_CONST,

    /*
    Lhs: Meta
    Rhs: Fields count
    Meta: Type, vis, doc tok, trailing fields
    */
    ENUM,

    /*
    Lhs: IDENT token
    Rhs: expression
    */
    ENUM_FIELD,

    /*
    Lhs: Meta
    Rhs: BLOCK
    Meta: expression, IF_STMT | BLOCK
    */
    IF_STMT,

    /*
    Lhs: Meta
    Rhs: BLOCK
    Meta: pattern, expression, label
    */
    FOR_STMT,

    /*
    Lhs: Meta
    Rhs: BLOCK
    Meta: expression, label
    */
    WHILE_STMT,

    /*
    Lhs: Meta
    Rhs: Size
    Meta: vis, doc tok, trailing nodes
    */
    TRAIT,

    /*
    Lhs: Type
    Rhs: Token
    Type:
        1 -> string
        2 -> char
        3 -> number
        4 -> float
    */
    LITERAL,

    /*
    Lhs: Meta
    Rhs: Size
    Meta:
        - traling nodes
    */
    ARRAY,

    /*
    Lhs: Size expression
    Rhs: expression
    */
    ARRAY_SHORT,

    /*
    Lhs: Meta
    Rhs: Size
    Meta:
        - trailing nodes
    */
    TUPLE,

    /*
    Lhs: None
    Rhs: expression
    */
    DISCARD,

    /*
    Lhs: None
    Rhs: expression
    */
    GROUPED_EXPR,

    /*
    Lhs: TYPE
    Rhs: expression
    */
    CAST_EXPR,

    /*
    Lhs: Meta
    Rhs: BLOCK
    Meta: CLOSURE_CAPTURES, FN_PARAMS, TYPE

    */
    CLOSURE_EXPR,

    /*
    Lhs: Size
    Rhs: End node
    */
    CLOSURE_CAPTURES,

    /*
    Lhs: flags [ MUTABLE, REFERENCE ]
    Rhs: expression
    */
    CLOSURE_CAPTURE,

    /*
    Lhs: meta value
    Rhs: TYPE_TUPLE, TYPE_ARR, TYPE_DYNAMIC, PATH_EXPR, TYPE_FN, TYPE_SLICE (slice)

    meta value: highest 3bits -> flags, lower 29bits -> count of pointers

    flags:
        1 -> REF
    */
    TYPE,

    // Same as the TYPE
    TYPE_SLICE,

    /*
    Lhs: TYPE,
    Rhs: expression
    */
    TYPE_ARR,

    /*
    Lhs: Meta
    Rhs: Size
    Meta:
        - trailing nodes
    */
    TYPE_TUPLE,

    /*
    Lhs: Meta
    Rhs: return type
    */
    TYPE_FN,

    /*
    Lhs: Meta
    Rhs: Size
    Meta:
        - trailing nodes
    */
    TYPE_DYNAMIC,

    /*
    Lhs: None
    Rhs: None
    */
    PATTERN_DISCARD,

    /*
    Lhs: TYPE
    Rhs: Is mutable
    MainToken: Ident token
    */
    PATTERN_SINGLE,

    /*
    Lhs: Meta
    Rhs: Size
    Meta: trailing pattern nodes
    */
    PATTERN_MULTIPLE,

    /*
    Lhs: PATH
    Rhs: BLOCK (contains multiple PATTERN_STRUCT_FIELD nodes)
    */
    PATTERN_STRUCT,

    /*
    Lhs: ident token (field name)
    Rhs: is mutable
    MainToken: Ident token(variable name)
    */
    PATTERN_STRUCT_FIELD,

    /*
    Lhs: SingleOp
    Rhs: expresion
    */
    SINGLE_OP,

    /*
    Lhs: None
    Rhs: expression
    */
    UNWRAP_OP,

    /*
    Lhs: expression
    Rhs: expression
    MainToken: enum Operators - CALL
    */
    BIN_OP,

    /*
    Lhs: Meta
    Rhs: expression (base)
    Meta: From, To
    */
    SLICE_OP,

    /*
    Lhs: Meta
    Rhs: expression
    Meta:
        - count
        - trailing expressions
    */
    CALL_OP,

    /*
    Lhs: FLOW
    Rhs: expression or label

    FLOW:
        1 -> break
        2 -> continue
        3 -> return
    */
    FLOW_OP,

    /*
    Lhs: PATH
    Rhs: STRUCT_EXPR_FIELDS or NULL_INDEX
    */
    STRUCT_EXPR,

    /*
    Lhs: Meta
    Rhs: Size
    Meta: trailing STRUCT_FIELD nodes
    */
    STRUCT_EXPR_FIELDS,

    /*
    Lhs: ident token
    Rhs: expression
    */
    STRUCT_EXPR_FIELD,

};

enum class SingleOp : Index {
    BOOL_NEG,
    BIT_NEG,
    GET_ADDR,
    INT_NEG,
    DEREF,
};

enum class Operators {
    INVALID = NULL_INDEX,
    CALL,
    ACCESS,
    ACCESS_PTR,
    ACCESS_ARR,
    UNWRAP,
    MUL,
    MOD,
    DIV,
    ADD,
    SUB,
    LS,
    GT,
    LS_EQ,
    GT_EQ,
    NOT_EQ,
    EQ_EQ,
    AND,
    OR_OR,
    AND_AND,
    OR,
    XOR,
    SHIFT_L,
    SHIFT_R,

    EQ,
    EQ_ADD,
    EQ_SUB,
    EQ_DIV,
    EQ_MUL,
    EQ_MOD,
    EQ_OR,
    EQ_AND,
    EQ_XOR,
};

constexpr Index TYPE_FLAGS_COUNT = 3;

constexpr Index TYPE_FLAGS_OFFSET = sizeof(Index) * 8 - TYPE_FLAGS_COUNT;
constexpr Index TYPE_PTR_MASK     = (1 << TYPE_FLAGS_OFFSET) - 1;
constexpr Index POINTER_MAX       = 8;

constexpr Index TYPE_FLAG_REFERENCE = 1 << TYPE_FLAGS_OFFSET;

constexpr Index LITERAL_STRING = 1;
constexpr Index LITERAL_CHAR   = 2;
constexpr Index LITERAL_NUMBER = 3;
constexpr Index LITERAL_FLOAT  = 4;

constexpr Index FLOW_BREAK    = 1;
constexpr Index FLOW_CONTINUE = 2;
constexpr Index FLOW_RETURN   = 3;

std::string_view tagToStr(Tag tag);

}
#endif // KOOLANG_ASTNODE_H
