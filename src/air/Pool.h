#ifndef KOOLANG_AIR_POOL_H
#define KOOLANG_AIR_POOL_H

#include "PoolKey.h"
#include "kir/RefInst.h"
#include "util/Index.h"
#include "util/StrCache.h"
#include "util/array_util.h"
#include <algorithm>
#include <array>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace air {

struct Module;

namespace pool::keys {
    constexpr auto NONE_KEY = PoolKey { KeyTag::NONE, {} };
    // types
    constexpr auto VOID_KEY           = PoolKey::CreateSimpleType(SimpleType::VOID);
    constexpr auto BOOL_KEY           = PoolKey::CreateSimpleType(SimpleType::BOOL);
    constexpr auto U8_KEY             = PoolKey::CreateSimpleType(SimpleType::U8);
    constexpr auto I8_KEY             = PoolKey::CreateSimpleType(SimpleType::I8);
    constexpr auto U16_KEY            = PoolKey::CreateSimpleType(SimpleType::U16);
    constexpr auto I16_KEY            = PoolKey::CreateSimpleType(SimpleType::I16);
    constexpr auto U32_KEY            = PoolKey::CreateSimpleType(SimpleType::U32);
    constexpr auto I32_KEY            = PoolKey::CreateSimpleType(SimpleType::I32);
    constexpr auto U64_KEY            = PoolKey::CreateSimpleType(SimpleType::U64);
    constexpr auto I64_KEY            = PoolKey::CreateSimpleType(SimpleType::I64);
    constexpr auto USIZE_KEY          = PoolKey::CreateSimpleType(SimpleType::USIZE);
    constexpr auto ISIZE_KEY          = PoolKey::CreateSimpleType(SimpleType::ISIZE);
    constexpr auto F16_KEY            = PoolKey::CreateSimpleType(SimpleType::F16);
    constexpr auto F32_KEY            = PoolKey::CreateSimpleType(SimpleType::F32);
    constexpr auto F64_KEY            = PoolKey::CreateSimpleType(SimpleType::F64);
    constexpr auto STR_KEY            = PoolKey::CreateSimpleType(SimpleType::STR);
    constexpr auto CHAR_KEY           = PoolKey::CreateSimpleType(SimpleType::CHAR);
    constexpr auto COMPTIME_INT_KEY   = PoolKey::CreateSimpleType(SimpleType::COMPTIME_INT);
    constexpr auto COMPTIME_FLOAT_KEY = PoolKey::CreateSimpleType(SimpleType::COMPTIME_FLOAT);

    // values
    constexpr auto ZERO_KEY     = PoolKey::CreateSimpleValue(SimpleValue::ZERO);
    constexpr auto ONE_KEY      = PoolKey::CreateSimpleValue(SimpleValue::ONE);
    constexpr auto NULL_PTR_KEY = PoolKey::CreateSimpleValue(SimpleValue::NULL_PTR);
    constexpr auto FALSE_KEY    = PoolKey::CreateSimpleValue(SimpleValue::BOOL_FALSE);
    constexpr auto TRUE_KEY     = PoolKey::CreateSimpleValue(SimpleValue::BOOL_TRUE);

    static constexpr auto UNSIGNED_INT_TYPES = std::to_array({
        U8_KEY,
        U16_KEY,
        U32_KEY,
        U64_KEY,
        USIZE_KEY,
    });

    static constexpr auto SIGNED_INT_TYPES = std::to_array({
        I8_KEY,
        I16_KEY,
        I32_KEY,
        I64_KEY,
        ISIZE_KEY,
    });

    static constexpr auto INT_TYPES = concatArrs(
        UNSIGNED_INT_TYPES, SIGNED_INT_TYPES,
        std::to_array({
            COMPTIME_INT_KEY,
        })
    );

    static constexpr auto FLOAT_TYPES = std::to_array({
        F16_KEY,
        F32_KEY,
        F64_KEY,
        COMPTIME_FLOAT_KEY,
    });

    static constexpr auto NUMERIC_TYPES = concatArrs(INT_TYPES, FLOAT_TYPES);

    static constexpr auto ALL_TYPES = concatArrs(
        std::to_array({
            NONE_KEY,
            VOID_KEY,
            BOOL_KEY,
            STR_KEY,
            CHAR_KEY,
        }),
        NUMERIC_TYPES
    );

    static constexpr auto ALL_VALUES = std::to_array({
        ZERO_KEY,
        ONE_KEY,
        NULL_PTR_KEY,
        TRUE_KEY,
        FALSE_KEY,
    });

    static constexpr auto ALL_KEYS = concatArrs(ALL_TYPES, ALL_VALUES);

    // NONE_KEY must be at the 0 index.
    static_assert(ALL_KEYS.at(0) == NONE_KEY);

    constexpr Index getKeyIndex(const PoolKey& key) {
        return static_cast<Index>(std::distance(ALL_KEYS.begin(), std::find(ALL_KEYS.begin(), ALL_KEYS.end(), key)));
    }

    constexpr Index COMPTIME_INT_INDEX   = getKeyIndex(COMPTIME_INT_KEY);
    constexpr Index COMPTIME_FLOAT_INDEX = getKeyIndex(COMPTIME_FLOAT_KEY);
    constexpr Index NONE_KEY_INDEX       = getKeyIndex(NONE_KEY);
    constexpr Index BOOL_KEY_INDEX       = getKeyIndex(BOOL_KEY);

    constexpr Index ZERO_VALUE_INDEX = 0;
    constexpr Index ONE_VALUE_INDEX  = 1;

    constexpr Index NULL_PTR_VALUE_INDEX = ZERO_VALUE_INDEX;
    constexpr Index FALSE_VALUE_INDEX    = ZERO_VALUE_INDEX;
    constexpr Index TRUE_VALUE_INDEX     = ONE_VALUE_INDEX;

    constexpr Index U8_KEY_INDEX    = getKeyIndex(U8_KEY);
    constexpr Index U16_KEY_INDEX   = getKeyIndex(U16_KEY);
    constexpr Index U32_KEY_INDEX   = getKeyIndex(U32_KEY);
    constexpr Index U64_KEY_INDEX   = getKeyIndex(U64_KEY);
    constexpr Index USIZE_KEY_INDEX = getKeyIndex(USIZE_KEY);
    constexpr Index I8_KEY_INDEX    = getKeyIndex(I8_KEY);
    constexpr Index I16_KEY_INDEX   = getKeyIndex(I16_KEY);
    constexpr Index I32_KEY_INDEX   = getKeyIndex(I32_KEY);
    constexpr Index I64_KEY_INDEX   = getKeyIndex(I64_KEY);
    constexpr Index ISIZE_KEY_INDEX = getKeyIndex(ISIZE_KEY);

}

class Pool {
public:
    // Returns the type of the kir constant
    static Index GetConstantType(kir::RefInst constant);

    // Returns the type and value of the kir constant
    static pool::TypeValue GetConstantTypeValue(kir::RefInst constant);

    // Checks if the index points at the constexpr key
    static constexpr bool IsKnownKey(Index poolIndex) { return poolIndex < pool::keys::ALL_KEYS.size(); }

    Pool();

    // Returns the index of data
    Index GetOrPut(const PoolKey& key);

    // Returns the index of data
    Index Put(const PoolKey& key);

    // Returns the index of data
    Index Get(const PoolKey& key) const;

    // Returns key data
    PoolKey::Ref GetKeyRef(Index dataIndex) const;

    // Deserializes the extra data
    template <typename Type> [[nodiscard]] Type GetExtra(Index extraIndex) const;

    // Returns index from which additional/extra data starts
    Index GetData(Index poolIndex) const { return m_data.at(poolIndex); }

    // Returns the type
    Index GetType(Index poolIndex) const;

    // Returns type and value. If the poolIndex points on the type then the type and value will be NONE_KEY_INDEX
    pool::TypeValue GetTypeValue(Index poolIndex) const;

    // Adds byte to the Bytes vector
    void AddByte(std::uint8_t byte);

    Index AddValue(std::uint64_t val);

    std::vector<std::uint8_t> Bytes;
    std::vector<std::uint64_t> Values;
    std::vector<std::string> Strings;

private:
    // strings lookup
    StrCache m_strCache;

    // serialized structs
    std::vector<Index> m_extra;

    // indexes to m_extra
    std::vector<Index> m_data;
    std::vector<pool::KeyTag> m_tags;

    std::unordered_map<PoolKey, Index, PoolKey::Hash> m_cache;

    // Serializes the data to m_extra vector
    template <typename Type> inline void AddToExtra(Type type) {
        const auto start = static_cast<Index>(m_extra.size());
        serializeToVec(m_extra, type);
        m_data.push_back(start);
    }
};

template <typename Type> [[nodiscard]] Type Pool::GetExtra(Index extraIndex) const {
    return deserializeFromVec<Type>(m_extra, extraIndex);
}

inline void Pool::AddByte(std::uint8_t byte) { Bytes.push_back(byte); }

inline Index Pool::AddValue(std::uint64_t val) {
    Values.push_back(val);
    return static_cast<Index>(Values.size() - 1);
}

}

#endif
