#ifndef KOOLANG_AIR_SEMA_H
#define KOOLANG_AIR_SEMA_H

#include "Inst.h"
#include "Pool.h"
#include "TypeInst.h"
#include "kir/Inst.h"
#include "symbol/Record.h"
#include "type.h"
#include "util/Index.h"

namespace air {

struct Module;

class Sema {
public:
    Sema(Module* mod, Air& air, Index kirInst, Index instCount);

    void AddSymbol();
    void Analyze();
    void AnalyzeDecl();
    void AnalyzeBody();

    static void PrepareModule(Module* mod);

    [[nodiscard]] Index GetKirInst() const { return m_kirInst; }
    [[nodiscard]] symbol::Record* GetRecord() const { return m_record; }
    [[nodiscard]] const Module* GetModule() const { return m_mod; }
    [[nodiscard]] const Air& GetAir() const { return m_air; }

private:
    Module* m_mod;
    Pool& m_pool;

    symbol::Record* m_record;
    Index m_blockInst;

    Index m_kirInstProcessing;

    kir::Kir& m_kir;

    // We can use reference because the vector will not resize.
    Air& m_air;

    const Index m_kirInst;
    const Index m_instCount;

    // kir -> air
    std::vector<Index> m_instMap;

    //-- HELPER METHODS -------------------------------------------------------------------------//

    // Creates constant instruction. Only the literal instructions can be passed to the template.
    template <kir::InstType> void CreateConstant(Index inst) = delete;

    // Derializes kir data so it's easier to work with.
    template <typename KirExtraData> KirExtraData GetKirData(Index extraIndex);

    // Creates constant instruction.
    void CreateConstantFrom(Index inst, Index poolIndex);

    // Creates instruction and maps kir instruction to the created air instruction.
    void CreateInst(InstType type, InstData data, Index kirInst);

    // Creates instruction and doesn't map the kir instruction.
    Index CreateInstNoMap(InstType type, InstData data);

    // Map the kir instruction to air instruction.
    void MapInst(Index kirInst, Index airInst) { m_instMap[GetRelativeKirIndex(kirInst)] = airInst; }

    // Returns relative index from the m_kirInst
    [[nodiscard]] Index GetRelativeKirIndex(Index kirInst) const;

    // Returns index of the air instruction, which is mapped to the given kir instruction.
    [[nodiscard]] Index GetLocalAirInst(Index kirInst) const { return m_instMap.at(GetRelativeKirIndex(kirInst)); }

    // Returns index to the intern pool
    Index GetAirType(Index airInst);

    // Returns index to the intern pool
    Index GetKirType(Index kirInst);

    // Returns type and instruction of the kir instruction. This method can create constant instruction if the kir
    // instruction is constant value.
    TypeInst GetTypeValue(kir::RefInst inst);

    // Returns symbol value and type
    TypeInst GetSymbolTypeValue(Index airInst);

    // Returns symbol type
    Index GetSymbolType(Index airInst);

    // Returns if the types was casted, e.g. (i8, i16) => true, (i8, StructABC) =>
    // false.
    bool TryCastSameType(TypeInst& valA, TypeInst& valB);

    bool IsConstant(Index airInst) { return m_air.Type.at(airInst) == InstType::CONSTANT; }
    bool AreConstants(Index airA, Index airB) { return IsConstant(airA) && IsConstant(airB); }

    //-- ANALYSIS METHODS -----------------------------------------------------------------------//

    void AnalyzeGlobDecl();

    void KirGlobConst(Index compBlockIndex);
    void KirGlobStatic(Index blockIndex);
    void KirGlobFnDecl();
    void KirGlobFnBody();
    void KirGlobEnum();
    void KirGlobStruct();
    void KirGlobVariant();

    //-- types ----------------------------------------------------------------------------------//
    void KirAs(Index inst);
    void KirAsAdvanced(kir::RefInst type, kir::RefInst val, Index inst);
    void KirAsConstant(Index typeIndex, kir::RefInst val, Index inst);

    //-- statements -----------------------------------------------------------------------------//
    void KirBreakInline(Index inst);

    //-- paths ----------------------------------------------------------------------------------//
    void KirDeclRef(Index inst);

    //-- arithmetic ops -------------------------------------------------------------------------//
    template <type::Operation Op> void KirArithmetic(Index inst);
    template <type::Operation Op> void EvalOp(Index inst, TypeInst lhs, TypeInst rhs);

    void AnalyzeBlock(Index blockIndex);
    void AnalyzeInst(Index inst);
};

inline void Sema::CreateConstantFrom(Index inst, Index poolIndex) {
    CreateInst(InstType::CONSTANT, InstData::CreatePoolIndex(poolIndex), inst);
}

inline Index Sema::GetRelativeKirIndex(Index kirInst) const {
    assert(kirInst >= (m_kirInst - m_instCount));
    return kirInst - (m_kirInst - m_instCount);
}

inline void Sema::CreateInst(InstType type, InstData data, Index kirInst) {
    MapInst(kirInst, CreateInstNoMap(type, data));
}

inline Index Sema::CreateInstNoMap(InstType type, InstData data) {
    m_air.Inst.push_back(data);
    m_air.Type.push_back(type);

    return static_cast<Index>(m_air.Inst.size() - 1);
}

template <typename KirExtraData> KirExtraData Sema::GetKirData(Index extraIndex) {
    return deserializeFromVec<KirExtraData>(m_kir.Extra, extraIndex);
}

}

#endif
