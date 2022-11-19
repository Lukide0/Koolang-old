#ifndef KOOLANG_TOKENLIST_H
#define KOOLANG_TOKENLIST_H

#include "Token.h"
#include "util/Index.h"
#include <array>
#include <cassert>
#include <string_view>
#include <vector>

namespace ast {

class TokenList {
public:
    std::string_view Code;
    std::vector<TokenTag> TokenTags;
    std::vector<TokenLoc> TokenLocs;

    TokenList() = default;
    TokenList(std::string_view sourceCode);

    // Gets the token at the index i
    [[nodiscard]] TokenTag GetTok(Index i);

    // Skip the token and returns skipped token tag.
    [[nodiscard]] TokenTag NextTok();

    // Skips token
    inline void SkipTok() { m_token++; }

    // Gets the token content
    [[nodiscard]] inline std::string_view GetTokContent(Index tok) const {
        return Code.substr(TokenLocs.at(tok).Start, TokenLocs.at(tok).Len);
    }

    // Gets the token start position
    [[nodiscard]] inline std::size_t GetTokStart(Index tok) const { return TokenLocs.at(tok).Start; }

    // Gets the token end position
    [[nodiscard]] inline std::size_t GetTokEnd(Index tok) const {
        return TokenLocs.at(tok).Len + TokenLocs.at(tok).Start;
    }

    // Checks token
    [[nodiscard]] inline bool Peek(TokenTag tag, Index offset = 0) const {
        return TokenTags.at(m_token + offset) == tag;
    }

    // Gets the current token index
    [[nodiscard]] inline Index GetCurrIndex() const { return m_token; }

    // Gets the current token tag
    [[nodiscard]] inline TokenTag GetCurrTok() const { return TokenTags.at(m_token); }

    // Skips token if the tag matches.
    bool Expect(TokenTag tag);

    // Skips token if the tag matches. All DOC_COMMENT tokens are skipped automatically. Returns the token index if tags
    // doesn't match, then returns NULL_INDEX.
    Index EatTok(TokenTag tag);

    // Skips token. All DOC_COMMENT tokens are skipped automatically. Returns the token index.
    Index EatTok();

    /**
     * Eat any token from given array. Skips all DOC_COMMENT tags.
     *
     * If the TokenTag doesn't match, then this function returns NULL_INDEX.
     */
    template <std::size_t Size> Index EatTokAny(const std::array<TokenTag, Size>& tags) {
        for (; TokenTags[m_token] == TokenTag::DOC_COMMENT; m_token++) { }

        Index index = NULL_INDEX;
        for (std::size_t i = 0; i < Size && isNull(index); i++) {
            index = (TokenTags.at(m_token) == tags[i]) ? m_token++ : NULL_INDEX;
        }

        return index;
    }

    /**
     * Eat documentation comments
     *
     * Returns index of the last encounter of DOC_COMMENT. If there isn't DOC_COMMENT, then return
     * NULL_INDEX.
     */
    Index EatDocComments();

private:
    Index m_token = 1;
};

}

#endif // KOOLANG_TOKENLIST_H
