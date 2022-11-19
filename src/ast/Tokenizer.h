#ifndef KOOLANG_TOKENIZER_H
#define KOOLANG_TOKENIZER_H

#include <array>
#include <vector>

#include "File.h"
#include "Token.h"
#include "TokenList.h"

namespace ast {

class Tokenizer {
public:
    Tokenizer(File& file);
    TokenList Tokenize();

private:
    TokenList m_tokens;
    File& m_file;
    std::string_view m_text;

    // Creates the token
    void inline InsertTok(TokenTag tag, TokenLoc::Pos start, TokenLoc::Pos len = 1);

    // States of the tokenizer
    enum State {
        NORMAL,
        IDENT,

        STRING_LIT,
        STRING_LIT_BACKSLASH,
        STRING_MULTILINE_LIT_LINE,
        STRING_MULTILINE_LIT_LINE_BACKSLASH,
        CHAR_LIT,
        CHAR_LIT_BACKSLASH,
        INT,
        INT_ZERO,
        INT_BIN,
        INT_HEX,
        INT_OCT,
        INT_PERIOD,
        FLOAT,
        DOC_COMMENT,
        COMMENT_BLOCK,
        COMMENT_BLOCK_START,

        MINUS,
        COLON,
        PLUS,
        STAR,
        MOD,
        SLASH,
        BANG,
        AND,
        OR,
        CARET,
        EQ,
        QUESTION,
    };
};

inline void Tokenizer::InsertTok(const TokenTag tag, const TokenLoc::Pos start, const TokenLoc::Pos len) {
    m_tokens.TokenTags.push_back(tag);
    m_tokens.TokenLocs.emplace_back(start, len);
}
}

#endif // KOOLANG_TOKENIZER_H
