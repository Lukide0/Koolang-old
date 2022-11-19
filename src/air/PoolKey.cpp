#include "PoolKey.h"
#include "util/hash_combine.h"

namespace air {

std::size_t PoolKey::Hash::operator()(const PoolKey& key) const {
    using pool::KeyTag;

    std::size_t hash = 0;
    switch (key.Tag) {
    case KeyTag::NONE:
        return 0;
    case KeyTag::TYPE_VALUE: {
        const auto tmp = key.Value.TyVal;
        hash_combine(hash, tmp.Ty, tmp.Val);
        break;
    }
    case KeyTag::ARR_TYPE: {
        const auto tmp = key.Value.ArrTy;
        hash_combine(hash, tmp.Ty, tmp.Len);
        break;
    }
    case KeyTag::SIMPLE_TYPE:
        hash_combine(hash, static_cast<int>(key.Value.SimpleTy));
        break;
    case KeyTag::SIMPLE_VALUE:
        hash_combine(hash, static_cast<int>(key.Value.SimpleVal));
        break;
    case KeyTag::BYTES: {
        const auto tmp = key.Value.Bytes;
        hash_combine(hash, tmp.Ty, tmp.ByteIndex);
        break;
    }
    case KeyTag::INT: {
        const auto tmp = key.Value.IntVal;
        hash_combine(hash, tmp.Ty, tmp.ValueIndex);
        break;
    }
    }

    return hash;
}

}
