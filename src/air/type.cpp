#include "type.h"
#include "Pool.h"

namespace air::type {

template <std::size_t N> inline bool knownKeysContains(const std::array<PoolKey, N>& keysArr, Index type) {
    using namespace pool;

    if (!Pool::IsKnownKey(type) || type == keys::NONE_KEY_INDEX) {
        return false;
    }

    return arrContains(keysArr, keys::ALL_KEYS.at(type));
}

template <pool::SimpleType... Types> inline bool isOneOf(pool::SimpleType type) { return ((type == Types) || ...); }
template <pool::SimpleType... Types> inline bool notOneOf(pool::SimpleType type) { return ((type != Types) && ...); }

bool isComptimeInt(Index type) { return type == pool::keys::COMPTIME_INT_INDEX; }
bool isIntType(Index type) { return knownKeysContains(pool::keys::INT_TYPES, type); }
bool isUnsignedInt(Index type) { return knownKeysContains(pool::keys::UNSIGNED_INT_TYPES, type); }
bool isSignedInt(Index type) { return knownKeysContains(pool::keys::SIGNED_INT_TYPES, type); }
bool isFloat(Index type) { return knownKeysContains(pool::keys::FLOAT_TYPES, type); }
bool isNumber(Index type) { return knownKeysContains(pool::keys::NUMERIC_TYPES, type); }

bool canCastInt(Index fromIntType, Index toIntType) {
    using pool::SimpleType;
    using pool::keys::ALL_KEYS;

    assert(isIntType(fromIntType) && isIntType(toIntType));

    if (fromIntType == toIntType || isComptimeInt(fromIntType)) {
        return true;
    }

    const SimpleType fromType = ALL_KEYS.at(fromIntType).Value.SimpleTy;
    const SimpleType toType   = ALL_KEYS.at(toIntType).Value.SimpleTy;

    switch (fromType) {
    case SimpleType::U8:
    case SimpleType::I8:
        return isOneOf<
            SimpleType::I16, SimpleType::U16, SimpleType::I32, SimpleType::U32, SimpleType::I64, SimpleType::U64>(toType
        );

    case SimpleType::U16:
    case SimpleType::I16:
        return isOneOf<SimpleType::I32, SimpleType::U32, SimpleType::I64, SimpleType::U64>(toType);

    case SimpleType::U32:
    case SimpleType::I32:
        return isOneOf<SimpleType::I64, SimpleType::U64>(toType);

    // usize, isize, u64 and i64 cannot be auto casted
    case SimpleType::U64:
    case SimpleType::I64:
    case SimpleType::USIZE:
    case SimpleType::ISIZE:
    default:
        return false;
    }
}

}
