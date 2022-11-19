#ifndef KOOLANG_AIR_POOLKEY_H
#define KOOLANG_AIR_POOLKEY_H

#include "util/Index.h"
#include "util/alias.h"
namespace air {
namespace pool {

    struct TypeValue {
        Index Ty;
        Index Val;
    };

    struct Int {
        Index Ty;
        Index ValueIndex;
    };

    struct Str {
        Index StrIndex;
    };

    struct ArrType {
        Index Ty;
        Index Len;
    };

    struct ByteStart {
        Index Ty;
        Index ByteIndex;
    };

    struct PtrType {
        enum Flags : Index {
            CONST = 1,
        };

        Index Ty;
        Flags Flags;
    };

    struct StructType {
        Index RecordIndex;
    };

    enum class SimpleType : Index {

        VOID = 1,
        BOOL,
        U8,
        I8,
        U16,
        I16,
        U32,
        I32,
        U64,
        I64,
        USIZE,
        ISIZE,
        F16,
        F32,
        F64,
        COMPTIME_INT,
        COMPTIME_FLOAT,
        CHAR,
        STR,
    };

    enum class SimpleValue : Index {
        ZERO = 1,
        ONE,

        NULL_PTR,

        BOOL_TRUE,
        BOOL_FALSE,
    };

    enum class KeyTag : std::uint8_t {
        NONE,
        // STORED ONLY IN DATA //
        SIMPLE_TYPE,
        SIMPLE_VALUE,
        // STORED IN EXTRA //
        BYTES,
        TYPE_VALUE,
        ARR_TYPE,
        INT,
    };

    union KeyValue {
        TypeValue TyVal;
        ArrType ArrTy;
        ByteStart Bytes;
        SimpleType SimpleTy;
        SimpleValue SimpleVal;
        Int IntVal;

        constexpr KeyValue()
            : IntVal({ 0, 0 }) { }

        constexpr KeyValue(TypeValue tyVal)
            : TyVal(tyVal) { }
        constexpr KeyValue(ArrType arrTy)
            : ArrTy(arrTy) { }
        constexpr KeyValue(ByteStart bytesVal)
            : Bytes(bytesVal) { }
        constexpr KeyValue(Int intValue)
            : IntVal(intValue) { }
        constexpr KeyValue(SimpleType simpleTy)
            : SimpleTy(simpleTy) { }

        constexpr KeyValue(SimpleValue simpleVal)
            : SimpleVal(simpleVal) { }
    };

    ASSERT_8BYTES(KeyValue);
}

struct PoolKey {

    pool::KeyTag Tag;
    pool::KeyValue Value;

    struct Ref {
        Index Extra;
        pool::KeyTag Tag;
    };

    ASSERT_8BYTES(Ref);

    struct Hash {
        std::size_t operator()(const PoolKey& key) const;
    };

    static constexpr PoolKey CreateSimpleType(pool::SimpleType type) { return { pool::KeyTag::SIMPLE_TYPE, type }; }
    static constexpr PoolKey CreateSimpleValue(pool::SimpleValue value) {
        return { pool::KeyTag::SIMPLE_VALUE, value };
    }
    static constexpr PoolKey CreateTypeValue(Index ty, Index val) {
        return { pool::KeyTag::TYPE_VALUE, pool::TypeValue { ty, val } };
    }
    static constexpr PoolKey CreateBytes(Index ty, Index begin) {
        return { pool::KeyTag::BYTES, pool::ByteStart { ty, begin } };
    }
    static constexpr PoolKey CreateIntVal(Index ty, Index valueIndex) {
        return { pool::KeyTag::INT, pool::Int { ty, valueIndex } };
    }

    constexpr bool operator==(const PoolKey& key) const {
        using pool::KeyTag;

        if (Tag != key.Tag) {
            return false;
        }

        bool areEqual;
        switch (Tag) {
        case KeyTag::NONE:
            areEqual = true;
            break;
        case KeyTag::SIMPLE_TYPE:
            areEqual = Value.SimpleTy == key.Value.SimpleTy;
            break;
        case KeyTag::SIMPLE_VALUE:
            areEqual = Value.SimpleVal == key.Value.SimpleVal;
            break;
        case KeyTag::BYTES:
            areEqual = Value.Bytes.Ty == key.Value.Bytes.Ty && Value.Bytes.ByteIndex == key.Value.Bytes.ByteIndex;
            break;
        case KeyTag::TYPE_VALUE:
            areEqual = Value.TyVal.Ty == key.Value.TyVal.Ty && Value.TyVal.Val == key.Value.TyVal.Val;
            break;
        case KeyTag::ARR_TYPE:
            areEqual = Value.ArrTy.Ty == key.Value.ArrTy.Ty && Value.ArrTy.Len == key.Value.ArrTy.Len;
            break;
        case KeyTag::INT:
            areEqual = Value.IntVal.Ty == key.Value.IntVal.Ty && Value.IntVal.ValueIndex == key.Value.IntVal.ValueIndex;
            break;
        }

        return areEqual;
    }
};

}

#endif
