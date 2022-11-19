#include "Tokenizer.h"
#include "codes/warn.h"
#include "logger/Record.h"
#include "util/ConstMap.h"
#include "util/debug.h"
#include <iostream>

namespace ast {

using namespace std::literals::string_view_literals;
using namespace logger;

// clang-format off
static constexpr auto TokenValues = std::to_array<std::pair<std::string_view, TokenTag>> ({
     { "import"sv, TokenTag::K_IMPORT },
     { "cast"sv, TokenTag::K_CAST },
     { "while"sv, TokenTag::K_WHILE },
     { "for"sv, TokenTag::K_FOR },
     { "if"sv, TokenTag::K_IF },
     { "else"sv, TokenTag::K_ELSE },
     { "const"sv, TokenTag::K_CONST },
     { "pub"sv, TokenTag::K_PUB },
     { "mut"sv, TokenTag::K_MUT },
     { "dyn"sv, TokenTag::K_DYN },
     { "static"sv, TokenTag::K_STATIC },
     { "new"sv, TokenTag::K_NEW },
     { "break"sv, TokenTag::K_BREAK },
     { "continue"sv, TokenTag::K_CONTINUE },
     { "return"sv, TokenTag::K_RETURN },
     { "struct"sv, TokenTag::K_STRUCT },
     { "trait"sv, TokenTag::K_TRAIT },
     { "enum"sv, TokenTag::K_ENUM },
     { "fn"sv, TokenTag::K_FN },
     { "variant"sv, TokenTag::K_VARIANT },
     { "impl"sv, TokenTag::K_IMPL },
     { "var"sv, TokenTag::K_VAR },
     { "in"sv, TokenTag::K_IN },
});
// clang-format on

bool inline isLetter(char letter) { return (letter >= 'a' && letter <= 'z') || (letter >= 'A' && letter <= 'Z'); }

bool inline isDigit(char digit) { return digit >= '0' && digit <= '9'; }

Tokenizer::Tokenizer(File& file)
    : m_tokens(file.Content)
    , m_file(file)
    , m_text(file.Content) { }

TokenList Tokenizer::Tokenize() {
    static constexpr auto keywords = ConstMap<std::string_view, TokenTag, TokenValues.size()> { { TokenValues } };

    const auto textSize = static_cast<TokenLoc::Pos>(m_text.size());
    auto state          = State::NORMAL;

    TokenLoc::Pos start = 0;
    TokenLoc::Pos index = 0;
    TokenTag tag        = TokenTag::INVALID;

    for (; index < textSize; index++) {
        const char character = m_text.at(index);

        switch (state) {
        case State::NORMAL:
            start = index;
            switch (character) {
            case '\n':
            case ' ':
            case '\t':
            case '\r':
            case '\0':
                break;
            case '(':
                InsertTok(TokenTag::PAREN_L, index);
                break;
            case ')':
                InsertTok(TokenTag::PAREN_R, index);
                break;
            case '[':
                InsertTok(TokenTag::SQUARE_L, index);
                break;
            case ']':
                InsertTok(TokenTag::SQUARE_R, index);
                break;
            case '{':
                InsertTok(TokenTag::CURLY_L, index);
                break;
            case '}':
                InsertTok(TokenTag::CURLY_R, index);
                break;
            case '<':
                InsertTok(TokenTag::LS, index);
                break;
            case '>':
                InsertTok(TokenTag::GT, index);
                break;
            case ';':
                InsertTok(TokenTag::SEMI, index);
                break;
            case '#':
                InsertTok(TokenTag::HASHTAG, index);
                break;
            case '.':
                InsertTok(TokenTag::DOT, index);
                break;
            case '~':
                InsertTok(TokenTag::TILDE, index);
                break;
            case ',':
                InsertTok(TokenTag::COMMA, index);
                break;
            case '?':
                state = State::QUESTION;
                tag   = TokenTag::QUESTION;
                break;
            case '_':
                state = State::IDENT;
                tag   = TokenTag::UNDERSCORE;
                break;
            case '"':
                state = State::STRING_LIT;
                tag   = TokenTag::STRING_LIT;
                break;
            case '\'':
                state = State::CHAR_LIT;
                tag   = TokenTag::CHAR_LIT;
                break;
            case '`':
                state = State::STRING_MULTILINE_LIT_LINE;
                tag   = TokenTag::STRING_LIT;
                break;
            case '/':
                state = State::SLASH;
                tag   = TokenTag::DIV;
                break;
            case '-':
                state = State::MINUS;
                tag   = TokenTag::MINUS;
                break;
            case '+':
                state = State::PLUS;
                tag   = TokenTag::ADD;
                break;
            case '*':
                state = State::STAR;
                tag   = TokenTag::STAR;
                break;
            case '%':
                state = State::MOD;
                tag   = TokenTag::MOD;
                break;
            case '!':
                state = State::BANG;
                tag   = TokenTag::BANG;
                break;
            case '&':
                state = State::AND;
                tag   = TokenTag::AND;
                break;
            case '|':
                state = State::OR;
                tag   = TokenTag::OR;
                break;
            case '^':
                state = State::CARET;
                tag   = TokenTag::CARET;
                break;
            case '=':
                state = State::EQ;
                tag   = TokenTag::EQ;
                break;
            case ':':
                state = State::COLON;
                tag   = TokenTag::COLON;
                break;
            default:
                // start of the identifier/keyword
                if (isLetter(character)) {
                    tag   = TokenTag::IDENT;
                    state = State::IDENT;
                } else if (character == '0') {
                    state = State::INT_ZERO;
                    tag   = TokenTag::NUMBER_LIT;
                } else if (isDigit(character)) {
                    state = State::INT;
                    tag   = TokenTag::NUMBER_LIT;
                }
                // unknown character, e.g. emoji
                else {
                    InsertTok(TokenTag::INVALID, start);
                }
                break;
            }
            break;
        // identifiers and keywords
        case State::IDENT: {
            char ch = character;
            while (isLetter(ch) || isDigit(ch) || ch == '_') {
                index += 1;
                if (index == textSize) {
                    break;
                }
                ch = m_text[index];
            }

            // check if the identifier is keyword
            tag = keywords.get(m_text.substr(start, index - start), tag);
            InsertTok(tag, start, index - start);

            state = State::NORMAL;
            index--;

            break;
        }
        // strings
        case State::STRING_LIT:
            switch (character) {
            case '"':
                state = State::NORMAL;
                InsertTok(tag, start, index - start);
                break;
            case '\\':
                state = State::STRING_LIT_BACKSLASH;
                break;
            case '\n':
                tag   = TokenTag::INVALID;
                state = State::STRING_LIT;
            default:
                break;
            }
            break;
        // multiline strings
        case State::STRING_LIT_BACKSLASH:
            switch (character) {
            case '\n':
                tag   = TokenTag::INVALID;
                state = State::STRING_LIT;
                break;
            default:
                state = State::STRING_LIT;
                break;
            }
            break;
        case State::STRING_MULTILINE_LIT_LINE:
            switch (character) {
            case '`':
                state = State::NORMAL;
                InsertTok(tag, start, index - start);
                break;
            case '\\':
                state = State::STRING_MULTILINE_LIT_LINE_BACKSLASH;
                break;
            default:
                break;
            }
            break;
        case State::STRING_MULTILINE_LIT_LINE_BACKSLASH:
            switch (character) {
            case '\n':
                tag   = TokenTag::INVALID;
                state = State::STRING_LIT;
                break;
            default:
                state = State::STRING_LIT;
                break;
            }
            break;
        case State::CHAR_LIT:
            switch (character) {
            case '\\':
                state = State::CHAR_LIT_BACKSLASH;
                break;
            case '\n':
                tag = TokenTag::INVALID;
                break;
            case '\'':
                state = State::NORMAL;
                InsertTok(tag, start, index - start);
                break;
            default:
                break;
            }
            break;
        case State::CHAR_LIT_BACKSLASH:
            if (character == '\n') {
                tag = TokenTag::INVALID;
            }

            state = State::CHAR_LIT;
            break;
        case State::INT:
            if (character == '.') {
                state = State::INT_PERIOD;
            } else if (character != '_' && !isDigit(character)) {
                InsertTok(tag, start, index - start);
                state = State::NORMAL;
                index--;
            }
            break;
        case State::INT_ZERO:
            switch (character) {
            case 'x':
                state = State::INT_HEX;
                break;
            case 'b':
                state = State::INT_BIN;
                break;
            case 'o':
                state = State::INT_OCT;
                break;
            default:
                index--;
                state = State::INT;
                break;
            }
            break;
        case State::INT_BIN:

            for (; m_text[index] == '0' || m_text[index] == '1' || m_text[index] == '_'; index++) {
                if (index + 1 == textSize) {
                    index++;
                    break;
                }
            }

            index--;
            InsertTok(TokenTag::NUMBER_LIT, start, index - start);
            state = State::NORMAL;
            break;
        case State::INT_HEX:
            // clang-format off
            for (; isDigit(m_text[index]) || (m_text[index] >= 'A' && m_text[index] <= 'F') || (m_text[index] >= 'a' && m_text[index] <= 'f'); index++)
            // clang-format on
            {
                if (index + 1 == textSize) {
                    index++;
                    break;
                }
            }

            InsertTok(TokenTag::NUMBER_LIT, start, index - start);
            index--;
            state = State::NORMAL;
            break;
        case State::INT_OCT:
            for (; m_text[index] >= '0' && m_text[index] <= '7'; index++) {
                if (index + 1 == textSize) {
                    index++;
                    break;
                }
            }

            index--;
            InsertTok(TokenTag::NUMBER_LIT, start, index - start);
            state = State::NORMAL;
            break;
        case State::INT_PERIOD:
            if (!isDigit(character)) {
                index -= 2;
                InsertTok(TokenTag::NUMBER_LIT, start, index - start - 2);
                state = State::NORMAL;
            } else {
                tag   = TokenTag::FLOAT_LIT;
                state = State::FLOAT;
            }

            break;
        case State::FLOAT:
            if (!isDigit(character) && character != '_') {
                InsertTok(TokenTag::FLOAT_LIT, start, index - start);
                state = State::NORMAL;
                index--;
            }
            break;
        case State::MINUS:
            switch (character) {
            case '=':
                InsertTok(TokenTag::MINUS_EQ, start);
                break;
            case '>':
                InsertTok(TokenTag::ARROW, start);
                break;
            default:
                InsertTok(TokenTag::MINUS, start);
                index--;
                break;
            }
            state = State::NORMAL;
            break;
        case State::QUESTION:
            if (character == '?') {
                InsertTok(TokenTag::QUESTION2, start, 1);
            } else {
                InsertTok(TokenTag::QUESTION, start);
                index--;
            }
            state = State::NORMAL;
            break;
        case State::COLON:
            if (character == ':') {
                InsertTok(TokenTag::COLON2, start, 1);
            } else {
                InsertTok(TokenTag::COLON, start);
                index--;
            }
            state = State::NORMAL;
            break;
        case State::PLUS:
            if (character == '=') {
                InsertTok(TokenTag::ADD_EQ, start, 1);
            } else {
                InsertTok(TokenTag::ADD, start);
                index--;
            }
            state = State::NORMAL;
            break;
        case State::STAR:
            if (character == '=') {
                InsertTok(TokenTag::STAR_EQ, start, 1);
            } else {
                InsertTok(TokenTag::STAR, start);
                index--;
            }
            state = State::NORMAL;
            break;
        case State::MOD:
            if (character == '=') {
                InsertTok(TokenTag::MOD_EQ, start, 1);
            } else {
                InsertTok(TokenTag::MOD, start);
                index--;
            }
            state = State::NORMAL;
            break;
        case State::BANG:
            if (character == '=') {
                InsertTok(TokenTag::NOT_EQ, start, 1);
            } else {
                InsertTok(TokenTag::BANG, start);
                index--;
            }
            state = State::NORMAL;
            break;
        case State::AND:
            if (character == '&') {
                InsertTok(TokenTag::AND_AND, start, 1);
            } else if (character == '=') {
                InsertTok(TokenTag::AND_EQ, start, 1);
            } else {
                InsertTok(TokenTag::AND, start);
                index--;
            }
            state = State::NORMAL;
            break;
        case State::OR:
            if (character == '|') {
                InsertTok(TokenTag::OR_OR, start, 1);
            } else if (character == '=') {
                InsertTok(TokenTag::OR_EQ, start, 1);
            } else {
                InsertTok(TokenTag::OR, start);
                index--;
            }
            state = State::NORMAL;
            break;
        case State::CARET:
            if (character == '=') {
                InsertTok(TokenTag::CARET_EQ, start, 1);
            } else {
                InsertTok(TokenTag::CARET, start);
                index--;
            }
            state = State::NORMAL;
            break;
        case State::EQ:
            if (character == '=') {
                InsertTok(TokenTag::EQ_EQ, start, 1);
            } else {
                InsertTok(TokenTag::EQ, start);
                index--;
            }
            state = State::NORMAL;
            break;
        case State::SLASH:
            switch (character) {
            case '*':
                state = State::COMMENT_BLOCK_START;
                break;
            case '=':
                state = State::NORMAL;
                InsertTok(TokenTag::DIV_EQ, start, 2);
                break;
            case '/':
                for (index++; index < textSize && m_text[index] != '\n'; index++) { }
                state = State::NORMAL;
                break;
            default:
                InsertTok(TokenTag::DIV, start);
                state = State::NORMAL;
                index--;
                break;
            }
            break;
        case State::COMMENT_BLOCK_START:

            if (index + 1 != textSize && character == '*') {
                if (m_text[index + 1] == '/') {
                    m_file.AddMsg(Record(
                        RecordKind::WARN, &m_file, codes::warn::EMPTY_MUTLTILINE_COMMENT, "Empty multiline comment",
                        Label("Remove this", Range { index - 2, index + 2 }).WithColor(Color::RED)
                    ));
                    state = State::NORMAL;
                    index++;
                    break;
                }
                tag   = TokenTag::DOC_COMMENT;
                state = State::DOC_COMMENT;
            } else {
                state = State::COMMENT_BLOCK;
            }
            break;
        case State::DOC_COMMENT:
            tag = TokenTag::INVALID;

            for (; index + 1 != textSize; index++) {
                if (m_text[index] == '*' && m_text[index + 1] == '/') {
                    index++;
                    tag = TokenTag::DOC_COMMENT;
                    break;
                }
            }

            InsertTok(tag, start, index - start);
            state = State::NORMAL;
            break;

        case State::COMMENT_BLOCK:
            tag = TokenTag::INVALID;

            for (; index + 1 != textSize; index++) {
                if (m_text[index] == '*' && m_text[index + 1] == '/') {
                    index += 1;
                    state = State::NORMAL;
                    break;
                }
            }

            if (state != State::NORMAL) {
                InsertTok(tag, start, index - start);
                state = State::NORMAL;
            }
            break;
        }
    }

    // after tokenizing the whole file, the last token may not be inserted
    switch (state) {
    case State::STRING_LIT:
    case State::STRING_LIT_BACKSLASH:
    case State::STRING_MULTILINE_LIT_LINE:
    case State::STRING_MULTILINE_LIT_LINE_BACKSLASH:
    case State::CHAR_LIT_BACKSLASH:
    case State::CHAR_LIT:
    case State::INT_BIN:
    case State::INT_HEX:
    case State::INT_OCT:
    case State::INT_PERIOD:
    case State::DOC_COMMENT:
    case State::COMMENT_BLOCK:
    case State::COMMENT_BLOCK_START:
        InsertTok(TokenTag::INVALID, start, index - 1 - start);
        break;
    case State::NORMAL:
        break;
    default:
        InsertTok(tag, start, index - 1 - start);
        break;
    }

    InsertTok(TokenTag::END_OF_FILE, textSize);

    return m_tokens;
}
}
