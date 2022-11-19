#ifndef KOOLANG_TOKEN_H
#define KOOLANG_TOKEN_H

#include <cstddef>
#include <cstdint>

namespace ast {

enum class TokenTag : std::uint8_t {
    // META
    INVALID,
    START_OF_FILE,
    END_OF_FILE,
    DOC_COMMENT, //  /** ... */

    // Brackets
    PAREN_L, // (
    PAREN_R, // )
    LS, // <
    GT, // >
    SQUARE_L, // [
    SQUARE_R, // ]
    CURLY_L, // {
    CURLY_R, // }

    STRING_LIT,
    CHAR_LIT,
    NUMBER_LIT,
    FLOAT_LIT,

    IDENT,

    UNDERSCORE, // _
    SEMI, // ;
    HASHTAG, // #
    ARROW, // ->
    DOT, // .
    COLON, // :
    COLON2, // ::
    COMMA, // ,

    // Operators
    ADD, // +
    ADD_EQ, // +=
    MINUS, // -
    MINUS_EQ, // -=
    STAR, // *
    STAR_EQ, // *=
    MOD, // %
    MOD_EQ, // %=
    DIV, // /
    DIV_EQ, // /=
    QUESTION, // ?
    QUESTION2, // ??

    BANG, // !
    TILDE, // ~

    AND, // &
    AND_AND, // &&
    AND_EQ, // &=
    OR, // |
    OR_OR, // ||
    OR_EQ, // |=
    CARET, // ^
    CARET_EQ, // ^=

    EQ, // =
    EQ_EQ, // ==
    NOT_EQ, // !=

    // Keywords
    K_IMPORT, // import

    K_CAST, // cast

    K_WHILE, // while
    K_FOR, // for

    K_IF, // if
    K_ELSE, // else

    K_CONST, // const
    K_PUB, // pub
    K_MUT, // mut
    K_DYN, // dyn
    K_STATIC, // static
    K_IN, // in

    K_NEW, // new

    K_BREAK, // break
    K_CONTINUE, // continue
    K_RETURN, // return

    K_STRUCT, // struct
    K_TRAIT, // trait
    K_ENUM, // enum
    K_VARIANT, // variant
    K_VAR, // var
    K_FN, // fn
    K_IMPL, // impl

    // TODO:
    /*
    extern - for external code (lib)
    switch/match
    await
    async
    assert
    test
    */

};

struct TokenLoc {
    using Pos = std::uint32_t;
    Pos Start;
    Pos Len;

    TokenLoc(Pos start = 0, Pos len = 0)
        : Start(start)
        , Len(len) { }
};

}

#endif // KOOLANG_TOKEN_H
