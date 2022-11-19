#include "TokenList.h"
#include "util/alias.h"

namespace ast {

TokenList::TokenList(std::string_view sourceCode)
    : Code(sourceCode) {
    TokenTags.reserve(TOKEN_BUFF_SIZE);
    TokenLocs.reserve(TOKEN_BUFF_SIZE);

    // Reserve 0-index
    TokenTags.push_back(TokenTag::START_OF_FILE);
    TokenLocs.emplace_back();
}

TokenTag TokenList::GetTok(Index i) {
    assert(i < TokenTags.size() && "Token index out of range");
    return TokenTags.at(i);
}

bool TokenList::Expect(TokenTag tag) {
    if (TokenTags.at(m_token) == tag) {
        m_token++;
        return true;
    } else {
        return false;
    }
}

TokenTag TokenList::NextTok() {
    assert(m_token < TokenTags.size() && "Token index out of range");
    return TokenTags[m_token++];
}

Index TokenList::EatTok(TokenTag tag) {
    for (; TokenTags[m_token] == TokenTag::DOC_COMMENT; m_token++) { }

    return (TokenTags[m_token] == tag) ? m_token++ : NULL_INDEX;
}

Index TokenList::EatTok() {
    for (; TokenTags[m_token] == TokenTag::DOC_COMMENT; m_token++) { }

    return m_token++;
}

Index TokenList::EatDocComments() {
    Index comment = (TokenTags[m_token] == TokenTag::DOC_COMMENT) ? m_token : NULL_INDEX;
    for (; comment != NULL_INDEX; comment = (TokenTags[m_token] == TokenTag::DOC_COMMENT) ? m_token : NULL_INDEX) {
        m_token++;
    }

    return comment;
}

}
