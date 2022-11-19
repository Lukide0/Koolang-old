#include "Scope.h"

namespace kir {

Scope::Scope(Type tag, Index name, Index meta, Index parent)
    : Tag(tag)
    , Meta(meta)
    , Name(name)
    , Parent(parent) { }

Index Scope::Contains(Index strIndex) const {
    if (NamedScopes.contains(strIndex)) {
        return NamedScopes.at(strIndex);
    } else {
        return NULL_INDEX;
    }
}

}
