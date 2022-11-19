#include "Sema.h"

namespace air {

void Sema::KirBreakInline(Index inst) {
    const auto& data = m_kir.Inst.at(inst).Bin;

    const Index blockInst = data.Lhs.Offset;
    const Index retInst   = data.Rhs.Offset;

    // end of the Sema
    if (m_blockInst == blockInst) {
        const Index airInst = GetLocalAirInst(retInst);
        const Index airType = GetAirType(airInst);

        m_record->Ty      = airType;
        m_record->AirInst = airInst;

        // previous instruction was constant
        if (IsConstant(airInst)) {
            m_record->Val = m_air.Inst.back().Data;
        }
    } else {
        KOOLANG_TODO();
    }
}
}
