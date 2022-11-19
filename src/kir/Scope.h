#ifndef KOOLANG_KIR_SCOPE_H
#define KOOLANG_KIR_SCOPE_H

#include "util/Index.h"
#include <cstdint>
#include <unordered_map>

namespace kir {

struct Scope {
    enum Type {
        TOP,
        BLOCK,
        SYMBOL,
    } Tag;

    std::unordered_map<Index, Index> NamedScopes;
    std::vector<Index> Scopes;

    Index Meta;
    Index Name;
    Index Parent;

    Scope(Type tag, Index name, Index meta = NULL_INDEX, Index parent = NULL_INDEX);

    Index Contains(Index strIndex) const;
};

struct SymbolMeta {
    static constexpr Index CONST_FLAG     = 1 << 0;
    static constexpr Index DISCARDED_FLAG = 1 << 1;

    Index Flags;
    Index Inst;

    SymbolMeta(Index inst = NULL_INDEX, Index flags = NULL_INDEX)
        : Flags(flags)
        , Inst(inst) { }
};

}

#endif
