#ifndef KOOLANG_UTIL_INDEX_H
#define KOOLANG_UTIL_INDEX_H

#include "debug.h"
#include <cassert>
#include <cstdint>
#include <vector>

using Index = std::uint32_t;

constexpr Index countBitsRequired(Index value) { return (value == 0) ? 0 : 1 + countBitsRequired(value >> 1); }

constexpr Index NULL_INDEX = 0;
constexpr Index MAX_INDEX  = ~static_cast<Index>(0);
constexpr int INDEX_BITS   = sizeof(Index) * 8;

constexpr int bitFlag(int bit) { return (bit == 0) ? 0 : 1 << (bit - 1); }

inline bool isNull(Index index) { return index == NULL_INDEX; }
template <typename T> inline bool isNull(T* ptr) { return ptr == nullptr; }

#define RETURN_IF_NULL(EXPR)           \
    if (isNull(EXPR)) {                \
        KOOLANG_ERR_MSG("NULL_INDEX"); \
        return NULL_INDEX;             \
    }

#define ASSERT_INDEX_SERIALIZABLE(TYPE)                                      \
    static_assert(                                                           \
        sizeof(TYPE) % sizeof(Index) == 0 && alignof(TYPE) == alignof(Index) \
        && std::has_unique_object_representations<TYPE>()                    \
    )

static_assert(NULL_INDEX == bitFlag(0));

#endif
