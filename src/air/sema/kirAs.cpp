#include "Sema.h"
#include "type.h"
#include "value.h"

namespace air {

using KirType = kir::InstType;
using KirData = kir::InstData;

void Sema::KirAs(Index inst) {
    const KirData& data = m_kir.Inst.at(inst);

    const kir::RefInst typeRef  = data.Bin.Lhs;
    const kir::RefInst valueRef = data.Bin.Rhs;

    KirAsAdvanced(typeRef, valueRef, inst);
}

void Sema::KirAsAdvanced(kir::RefInst type, kir::RefInst val, Index inst) {
    // Both KIR instructions must point to AIR instruction which is already resolved and is in m_instMap
    const Index typeIndex = (type.IsConstant()) ? Pool::GetConstantType(type) : GetKirType(type.Offset);

    if (isNull(typeIndex)) {
        KOOLANG_WARN_MSG("NULL TYPE");
        return;
    }

    // check if the value is known constant (0, 1, false, true, null)
    if (val.IsConstant()) {
        KirAsConstant(typeIndex, val, inst);
        return;
    }

    const Index airValueInst   = GetLocalAirInst(val.Offset);
    const Index valueTypeIndex = GetAirType(airValueInst);

    // Check comptime int
    if (type::isComptimeInt(valueTypeIndex)) {
        if (!type::isIntType(typeIndex)) {
            KOOLANG_ERR_MSG("MISMATCHED TYPES");
            return;
        }

        // we know that the val instruction is constant with comptime integer
        const Index poolIndex  = m_air.Inst.at(airValueInst).Data;
        const Index extraIndex = m_pool.GetData(poolIndex);
        const Index valueIndex = m_pool.GetExtra<pool::TypeValue>(extraIndex).Val;

        if (!value::canFitInt(typeIndex, m_pool.Values.at(valueIndex))) {
            KOOLANG_ERR_MSG("CANNOT FIT INT");
            return;
        }

        CreateConstantFrom(inst, m_pool.GetOrPut(PoolKey::CreateTypeValue(typeIndex, valueIndex)));
    }
    // check ints
    else if (type::isIntType(typeIndex)) {
        if (!type::isIntType(valueTypeIndex)) {
            KOOLANG_ERR_MSG("MISMATCHED TYPES");
            return;
        }

        // check if we don't need cast instruction
        if (type::areSame(typeIndex, valueTypeIndex)) {
            MapInst(inst, airValueInst);
            return;
        }

        if (!type::canCastInt(valueTypeIndex, typeIndex)) {
            KOOLANG_ERR_MSG("CANNOT CAST INT");
            return;
        }

        if (IsConstant(airValueInst)) {
            const Index poolIndex     = m_air.Inst.at(airValueInst).Data;
            const Index extraIndex    = m_pool.GetData(poolIndex);
            const Index valueKeyIndex = m_pool.GetExtra<pool::Int>(extraIndex).ValueIndex;

            CreateConstantFrom(inst, m_pool.GetOrPut(PoolKey::CreateTypeValue(typeIndex, valueKeyIndex)));
            return;
        }

        // TODO: Create cast int instruction
        KOOLANG_TODO();
        KOOLANG_CUSTOM_MSG("  FROM", "{}", valueTypeIndex);
        KOOLANG_CUSTOM_MSG("  TO {}", "{}", typeIndex);
    }
}

void Sema::KirAsConstant(Index typeIndex, kir::RefInst val, Index inst) {
    using pool::keys::ONE_VALUE_INDEX;
    using pool::keys::ZERO_VALUE_INDEX;

    if (!val.IsValue()) {
        KOOLANG_ERR_MSG("NOT VALID VALUE");
        return;
    }

    const Index valTypeIndex = Pool::GetConstantType(val);

    // 0 or 1
    if (type::isComptimeInt(valTypeIndex)) {
        const Index valueKeyIndex = (val.ToConstant() == kir::RefInst::ZERO) ? ZERO_VALUE_INDEX : ONE_VALUE_INDEX;
        CreateConstantFrom(inst, m_pool.GetOrPut(PoolKey::CreateTypeValue(typeIndex, valueKeyIndex)));
    }

    else {
        KOOLANG_TODO();
    }
}

}
