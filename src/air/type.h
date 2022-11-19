#ifndef KOOLANG_AIR_TYPE_H
#define KOOLANG_AIR_TYPE_H

#include "Pool.h"
#include "util/Index.h"

namespace air::type {

enum class Operation {
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,

    BIT_AND,
    BIT_OR,
    BIT_SHL,
    BIT_SHR,
    BIT_XOR,
};

template <Operation Op> struct operation {
    static constexpr auto VALUE = Op;
};

bool isComptimeInt(Index type);
bool isIntType(Index type);
bool isUnsignedInt(Index type);
bool isSignedInt(Index type);
bool isFloat(Index type);
bool isNumber(Index type);

bool canCastInt(Index fromIntType, Index toIntType);
inline bool areSame(Index typeA, Index typeB) { return typeA == typeB; }

// Returns if the type is number(signed, unsigned, float), string, char or bool.
inline bool isPrimitive(Index type) { return Pool::IsKnownKey(type); }

}

#endif
