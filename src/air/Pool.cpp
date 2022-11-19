#include "Pool.h"
#include "kir/RefInst.h"
#include "util/alias.h"
#include "util/array_util.h"

namespace air {

Pool::Pool()
    : m_strCache(Strings) {
    m_cache.emplace(pool::keys::NONE_KEY, NULL_INDEX);
    m_tags.push_back(pool::KeyTag::NONE);
    m_data.push_back(NULL_INDEX);

    Values.push_back(0);
    Values.push_back(1);

    assert(Values.at(pool::keys::ZERO_VALUE_INDEX) == 0);
    assert(Values.at(pool::keys::ONE_VALUE_INDEX) == 1);

    for (const auto& key : pool::keys::ALL_KEYS) {
        DISCARD_VALUE(Put(key));
    }
}

Index Pool::Get(const PoolKey& key) const {
    auto iter = m_cache.find(key);

    if (iter != m_cache.end()) {
        return iter->second;
    } else {
        return NULL_INDEX;
    }
}

Index Pool::GetOrPut(const PoolKey& key) {
    Index tmp = Get(key);
    if (isNull(tmp)) {
        tmp = Put(key);
    }

    return tmp;
}

Index Pool::Put(const PoolKey& key) {
    if (key.Tag == pool::KeyTag::NONE) {
        return NULL_INDEX;
    }

    m_tags.push_back(key.Tag);
    switch (key.Tag) {
    case pool::KeyTag::SIMPLE_TYPE:
        m_data.push_back(static_cast<Index>(key.Value.SimpleTy));
        break;
    case pool::KeyTag::SIMPLE_VALUE:
        m_data.push_back(static_cast<Index>(key.Value.SimpleVal));
        break;
    case pool::KeyTag::TYPE_VALUE:
        AddToExtra(key.Value.TyVal);
        break;
    case pool::KeyTag::ARR_TYPE:
        AddToExtra(key.Value.ArrTy);
        break;
    case pool::KeyTag::BYTES:
        AddToExtra(key.Value.Bytes);
        break;
    case pool::KeyTag::INT:
        AddToExtra(key.Value.IntVal);
        break;

    default:
        KOOLANG_UNREACHABLE();
    }

    const auto dataIndex = static_cast<Index>(m_data.size() - 1);

    m_cache.insert({ key, dataIndex });

    return dataIndex;
}

PoolKey::Ref Pool::GetKeyRef(Index dataIndex) const {
    return PoolKey::Ref { GetData(dataIndex), m_tags.at(dataIndex) };
}

Index Pool::GetType(Index poolIndex) const {
    using pool::KeyTag;

    Index typeIndex = pool::keys::NONE_KEY_INDEX;

    switch (m_tags.at(poolIndex)) {
    case pool::KeyTag::SIMPLE_TYPE:
        typeIndex = poolIndex;
        break;
    case pool::KeyTag::BYTES:
        typeIndex = deserializeFromVec<pool::ByteStart>(m_extra, GetData(poolIndex)).Ty;
        break;
    case pool::KeyTag::TYPE_VALUE:
        typeIndex = deserializeFromVec<pool::TypeValue>(m_extra, GetData(poolIndex)).Ty;
        break;
    case pool::KeyTag::ARR_TYPE:
        break;
    case pool::KeyTag::INT:
        typeIndex = deserializeFromVec<pool::Int>(m_extra, GetData(poolIndex)).Ty;
        break;
    case pool::KeyTag::NONE:
    case pool::KeyTag::SIMPLE_VALUE:
        KOOLANG_UNREACHABLE();
    }

    return typeIndex;
}

pool::TypeValue Pool::GetTypeValue(Index poolIndex) const {
    pool::TypeValue tyVal { pool::keys::NONE_KEY_INDEX, pool::keys::NONE_KEY_INDEX };

    switch (m_tags.at(poolIndex)) {
    case pool::KeyTag::NONE:
    case pool::KeyTag::SIMPLE_TYPE:
        break;
    case pool::KeyTag::SIMPLE_VALUE:

    case pool::KeyTag::BYTES:
    case pool::KeyTag::TYPE_VALUE:
    case pool::KeyTag::ARR_TYPE:
        break;
    case pool::KeyTag::INT: {
        const auto data = deserializeFromVec<pool::Int>(m_extra, m_data.at(poolIndex));
        tyVal.Ty        = data.Ty;
        tyVal.Val       = data.ValueIndex;
        break;
    }
    }

    return tyVal;
}

Index Pool::GetConstantType(kir::RefInst constant) {
    assert(constant.IsConstant());

    const kir::RefInst::Constants type = constant.ToConstant();
    PoolKey key;

    switch (type) {
    case kir::RefInst::NONE:
        key = pool::keys::NONE_KEY;
        break;
    case kir::RefInst::ZERO:
    case kir::RefInst::ONE:
        key = pool::keys::COMPTIME_INT_KEY;
        break;
    case kir::RefInst::NULL_VALUE:
        key = pool::keys::NULL_PTR_KEY;
        break;
    case kir::RefInst::VOID_TYPE:
        key = pool::keys::VOID_KEY;
        break;
    case kir::RefInst::BOOL_TRUE:
    case kir::RefInst::BOOL_FALSE:
    case kir::RefInst::BOOL_TYPE:
        key = pool::keys::BOOL_KEY;
        break;
    case kir::RefInst::U8_TYPE:
        key = pool::keys::U8_KEY;
        break;
    case kir::RefInst::I8_TYPE:
        key = pool::keys::I8_KEY;
        break;
    case kir::RefInst::U16_TYPE:
        key = pool::keys::U16_KEY;
        break;
    case kir::RefInst::I16_TYPE:
        key = pool::keys::I16_KEY;
        break;
    case kir::RefInst::U32_TYPE:
        key = pool::keys::U32_KEY;
        break;
    case kir::RefInst::I32_TYPE:
        key = pool::keys::I32_KEY;
        break;
    case kir::RefInst::U64_TYPE:
        key = pool::keys::U64_KEY;
        break;
    case kir::RefInst::I64_TYPE:
        key = pool::keys::I64_KEY;
        break;
    case kir::RefInst::USIZE_TYPE:
        key = pool::keys::USIZE_KEY;
        break;
    case kir::RefInst::ISIZE_TYPE:
        key = pool::keys::ISIZE_KEY;
        break;
    case kir::RefInst::F16_TYPE:
        key = pool::keys::F16_KEY;
        break;
    case kir::RefInst::F32_TYPE:
        key = pool::keys::F32_KEY;
        break;
    case kir::RefInst::F64_TYPE:
        key = pool::keys::F64_KEY;
        break;
    case kir::RefInst::STR_TYPE:
        key = pool::keys::STR_KEY;
        break;
    case kir::RefInst::CHAR_TYPE:
        key = pool::keys::CHAR_KEY;
        break;
    }

    return pool::keys::getKeyIndex(key);
}

pool::TypeValue Pool::GetConstantTypeValue(kir::RefInst constant) {
    assert(constant.IsConstant());

    pool::TypeValue tyVal { NULL_INDEX, NULL_INDEX };
    if (!constant.IsValue()) {
        return tyVal;
    }

    const kir::RefInst::Constants type = constant.ToConstant();

    switch (type) {
    case kir::RefInst::ZERO:
        tyVal.Val = pool::keys::ZERO_VALUE_INDEX;
        tyVal.Ty  = pool::keys::COMPTIME_INT_INDEX;
        break;
    case kir::RefInst::ONE:
        tyVal.Val = pool::keys::ONE_VALUE_INDEX;
        tyVal.Ty  = pool::keys::COMPTIME_INT_INDEX;
        break;
    case kir::RefInst::NULL_VALUE:
        tyVal.Val = pool::keys::NULL_PTR_VALUE_INDEX;
        KOOLANG_TODO();
        tyVal.Ty = pool::keys::getKeyIndex(pool::keys::NULL_PTR_KEY);
        break;
    case kir::RefInst::BOOL_TRUE:
        tyVal.Val = pool::keys::TRUE_VALUE_INDEX;
        tyVal.Ty  = pool::keys::BOOL_KEY_INDEX;
        break;
    case kir::RefInst::BOOL_FALSE:
        tyVal.Val = pool::keys::FALSE_VALUE_INDEX;
        tyVal.Ty  = pool::keys::BOOL_KEY_INDEX;
        break;
    default:
        break;
    }

    return tyVal;
}
}
