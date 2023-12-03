#include "Sema.h"
#include "Module.h"
#include "Printer.h"
#include "ast/Vis.h"
#include "kir/Extra.h"
#include "kir/Inst.h"
#include "type.h"
#include "util/array_util.h"
#include "value.h"
#include <span>

namespace air {

using KirType = kir::InstType;
using KirData = kir::InstData;

Sema::Sema(Module* mod, Air& air, Index kirInst, Index instCount)
    : m_mod(mod)
    , m_pool(mod->InternPool)
    , m_kir(mod->Kir)
    , m_air(air)
    , m_kirInst(kirInst)
    , m_instCount(instCount) {
    m_instMap.resize(m_instCount);

    // reserve zero index
    m_air.Type.emplace_back();
    m_air.Inst.emplace_back(NULL_INDEX);
}

void Sema::PrepareModule(Module* mod) {
    const Index topDeclsExtra = mod->Kir.Inst.at(1).NodePl.Payload.Offset;
    const Index topDeclsCount = mod->Kir.Extra.at(topDeclsExtra);

    mod->Airs.reserve(topDeclsCount);
    mod->Semas.reserve(topDeclsCount);

    // NULL + TOP BLOCK
    Index startOffset = 2;
    for (Index i = 1; i <= topDeclsCount; i++) {
        const Index instOffset = mod->Kir.Extra.at(topDeclsExtra + i);

        Sema& sema = mod->Semas.emplace_back(mod, mod->Airs.emplace_back(), instOffset, instOffset - startOffset);
        sema.AddSymbol();

        startOffset = instOffset;
    }
}

void Sema::AddSymbol() {
    const auto& decl = m_kir.Inst.at(m_kirInst).Bin;
    const auto& meta = deserializeFromVec<kir::extra::Decl>(m_kir.Extra, decl.Lhs.Offset);
    const auto vis   = static_cast<ast::Vis>(meta.Vis);

    m_record = m_mod->Map.CreateRecord(
        m_mod->NamespaceIndex, m_kir.Strings.at(meta.Name), vis, m_kirInst, NULL_INDEX, m_mod
    );
}

template <> void Sema::CreateConstant<KirType::INT>(const Index inst) {
    const KirData& data = m_kir.Inst.at(inst);

    const Index poolIndex
        = m_pool.Put(PoolKey::CreateTypeValue(pool::keys::COMPTIME_INT_INDEX, m_pool.AddValue(data.Int)));
    CreateConstantFrom(inst, poolIndex);
}

template <> void Sema::CreateConstant<KirType::FLOAT>(const Index inst) {
    const KirData& data = m_kir.Inst.at(inst);

    const Index poolIndex = m_pool.Put(PoolKey::CreateTypeValue(
        pool::keys::COMPTIME_FLOAT_INDEX, m_pool.AddValue(reinterpret_cast<const std::uint64_t&>(data.Float))
    ));

    CreateConstantFrom(inst, poolIndex);
}

void Sema::AnalyzeGlobDecl() {
    using symbol::Record;

    m_record->StatusBody = Record::State::IN_PROGRESS;

    const auto& decl = m_kir.Inst.at(m_kirInst).Bin;
    m_blockInst      = decl.Rhs.Offset;

    if (m_kir.Type.at(decl.Rhs.Offset) == KirType::BLOCK_COMPTIME_INLINE) {
        KirGlobConst(decl.Rhs.Offset);
    } else {
        KirGlobStatic(decl.Rhs.Offset);
    }

    freeVecMemory(m_instMap);
    m_record->StatusBody = Record::State::COMPLETE;
}

void Sema::AnalyzeDecl() {
    using symbol::Record;

    if (m_record->StatusDecl != Record::State::NOT_ANALYZED) {
        return;
    }

    m_record->StatusDecl = Record::State::IN_PROGRESS;

    const KirType type = m_kir.Type.at(m_kirInst);
    switch (type) {
    // We need decl + body
    case KirType::DECL:
        AnalyzeGlobDecl();
        break;
    case KirType::DECL_FN:
        KirGlobFnDecl();
        break;
    case KirType::DECL_ENUM:
        break;
    case KirType::DECL_STRUCT:
        break;
    case KirType::DECL_VARIANT:
        break;
    default:
        KOOLANG_UNREACHABLE();
    }
    m_record->StatusDecl = Record::State::COMPLETE;
}

void Sema::AnalyzeBody() {
    using symbol::Record;

    if (m_record->StatusBody != Record::State::NOT_ANALYZED) {
        return;
    }

    m_record->StatusBody = Record::State::IN_PROGRESS;

    const KirType type = m_kir.Type.at(m_kirInst);
    switch (type) {
    // We need decl + body
    case KirType::DECL:
        AnalyzeGlobDecl();
        break;
    case KirType::DECL_FN:
        KirGlobFnDecl();
        KirGlobFnBody();
        break;
    case KirType::DECL_ENUM:
        break;
    case KirType::DECL_STRUCT:
        break;
    case KirType::DECL_TRAIT:
        break;
    case KirType::DECL_IMPL:
        break;

    case KirType::DECL_VARIANT:
        break;
    default:
        KOOLANG_UNREACHABLE();
    }

    m_record->StatusBody = Record::State::COMPLETE;
}

void Sema::Analyze() {
    AnalyzeDecl();
    AnalyzeBody();

    freeVecMemory(m_instMap);
    // TODO: REMOVE
    Printer(this).Print();
}

void Sema::AnalyzeBlock(Index blockIndex) {
    const auto block           = m_kir.Inst.at(blockIndex).NodePl;
    const auto blockExtraIndex = block.Payload.Offset;

    const auto itemsCount = m_kir.Extra.at(blockExtraIndex);

    std::span<Index> instructions { m_kir.Extra.begin() + blockExtraIndex + 1, itemsCount };

    for (const Index inst : instructions) {
        AnalyzeInst(inst);
    }
}

void Sema::AnalyzeInst(Index inst) {
    const KirType type = m_kir.Type.at(inst);

    switch (type) {
    // instruction without type
    case KirType::NONE:
    case KirType::IDENT:
        break;
    // memory instructions
    case KirType::ALLOC:
    case KirType::ALLOC_MUT:
    case KirType::ALLOC_INFERRED:
    case KirType::ALLOC_MUT_INFERRED:
    case KirType::STORE:
    case KirType::STORE_INFERRED:
    case KirType::STORE_NODE:
    case KirType::LOAD:
        KOOLANG_INFO_MSG("MEMORY");
        break;
    // path instructions
    case KirType::DECL_REF:
        KirDeclRef(inst);
        break;
    case KirType::DECL_ITEM:
    case KirType::NAMESPACE:
        KOOLANG_INFO_MSG("PATH");
        break;
    // statements
    case KirType::BREAK_INLINE:
        KirBreakInline(inst);
        break;

    case KirType::BLOCK:
    case KirType::LOOP:
    case KirType::BLOCK_INLINE:
    case KirType::BLOCK_COMPTIME_INLINE:
    case KirType::BREAK:
    case KirType::RETURN:
    case KirType::CONTINUE:
    case KirType::GOTO:
    case KirType::REPEAT:
    case KirType::CONDBR:
        KOOLANG_INFO_MSG("STATEMENT");
        break;
    // logic operations
    case KirType::LOGIC_OR:
    case KirType::LOGIC_AND:
        KOOLANG_INFO_MSG("LOGIC");
        break;
    // discard operation
    case KirType::DISCARD_DESTRUCTOR:
        KOOLANG_INFO_MSG("DISCARD");
        break;
    // array operations
    case KirType::INDEXABLE_LEN:
    case KirType::ARR_INIT:
    case KirType::ARR_SHORT_INIT:
        KOOLANG_INFO_MSG("ARRAY");
        break;
    // tuple
    case KirType::TUPLE:
        KOOLANG_INFO_MSG("TUPLE");
        break;
    // call instruction
    case KirType::CALL:
        KOOLANG_INFO_MSG("CALL");
        break;
    // literals
    case KirType::INT:
        CreateConstant<KirType::INT>(inst);
        break;
    case KirType::FLOAT:
        CreateConstant<KirType::FLOAT>(inst);
        break;
    case KirType::STR:
        // CreateConstant<KirType::STR>(inst);
        // break;
    case KirType::CHAR:
        // CreateConstant<KirType::CHAR>(inst);
        // break;
    // field access (struct, tuple, ...)
    case KirType::FIELD:
    case KirType::FIELD_SHORT:
        KOOLANG_INFO_MSG("FIELD");
        break;

    // arithmetic operations
    case KirType::ADD:
        KirArithmetic<type::Operation::ADD>(inst);
        break;
    case KirType::SUB:
        KirArithmetic<type::Operation::SUB>(inst);
        break;
    case KirType::MUL:
        KirArithmetic<type::Operation::MUL>(inst);
        break;
    case KirType::DIV:
        KirArithmetic<type::Operation::DIV>(inst);
        break;
    case KirType::MOD:
        KirArithmetic<type::Operation::MOD>(inst);
        break;
    case KirType::BIT_AND:
        KirArithmetic<type::Operation::BIT_AND>(inst);
        break;
    case KirType::BIT_OR:
        KirArithmetic<type::Operation::BIT_OR>(inst);
        break;
    case KirType::BIT_SHL:
        KirArithmetic<type::Operation::BIT_SHL>(inst);
        break;
    case KirType::BIT_SHR:
        KirArithmetic<type::Operation::BIT_SHR>(inst);
        break;
    case KirType::BIT_XOR:
        KirArithmetic<type::Operation::BIT_XOR>(inst);
        break;
    case KirType::ARR_EL:
    case KirType::CMP_LS:
    case KirType::CMP_GT:
    case KirType::CMP_LSE:
    case KirType::CMP_GTE:
    case KirType::CMP_EQ:
    case KirType::CMP_NEQ:
        KOOLANG_INFO_MSG("ARITHMETIC");
        break;
    // slice
    case KirType::SLICE_FULL:
    case KirType::SLICE_START:
    case KirType::SLICE_END:
        KOOLANG_INFO_MSG("SLICE");
        break;
    // struct
    case KirType::STRUCT_INIT_EMPTY:
    case KirType::STRUCT_INIT:
        KOOLANG_INFO_MSG("STRUCT");
        break;
    // unary operations
    case KirType::BOOL_NEG:
    case KirType::BIT_NEG:
    case KirType::GET_ADDR:
    case KirType::DEREF:
    case KirType::INT_NEG:
    case KirType::UNWRAP:
        KOOLANG_INFO_MSG("UNARY");
        break;
    // type instructions
    case KirType::AS:
        KirAs(inst);
        break;
    case KirType::SLICE_TYPE:
    case KirType::ARRAY_TYPE:
    case KirType::PTR_TYPE:
    case KirType::TUPLE_TYPE:
    case KirType::DYN_TYPE:
    case KirType::REF_TYPE:
    case KirType::CAST:
        KOOLANG_INFO_MSG("TYPE");
        break;
    // these instructions shouldn't appear
    case KirType::DECL:
    case KirType::DECL_FN:
    case KirType::DECL_ENUM:
    case KirType::DECL_STRUCT:
    case KirType::DECL_VARIANT:
    case KirType::DECL_TRAIT:
    case KirType::DECL_IMPL:
    case KirType::STRUCT_FIELD:
    case KirType::ENUM_FIELD:
    case KirType::PARAM:
        KOOLANG_UNREACHABLE();
    }
}

Index Sema::GetKirType(Index kirInst) {
    const Index airInst = GetLocalAirInst(kirInst);
    if (isNull(airInst)) {
        KOOLANG_ERR_MSG("NULL INST");
    }
    return GetAirType(airInst);
}

TypeInst Sema::GetTypeValue(kir::RefInst inst) {
    TypeInst tyInst { NULL_INDEX, NULL_INDEX };

    if (inst.IsConstant()) {
        const auto tyVal = Pool::GetConstantTypeValue(inst);

        // the constant is type
        if (tyVal.Ty == pool::keys::NONE_KEY_INDEX) {
            KOOLANG_ERR_MSG("EXPECTED VALUE FOUND TYPE");
            return tyInst;
        }

        tyInst.Ty = tyVal.Ty;
        tyInst.Inst
            = CreateInstNoMap(InstType::CONSTANT, m_pool.GetOrPut({ pool::KeyTag::TYPE_VALUE, std::move(tyVal) }));

        return tyInst;
    }

    tyInst.Inst = GetLocalAirInst(inst.Offset);
    tyInst.Ty   = GetAirType(tyInst.Inst);

    return tyInst;
}

Index Sema::GetAirType(Index airInst) {
    Index poolIndex;

    switch (m_air.Type.at(airInst)) {
    case InstType::CONSTANT:
        poolIndex = m_air.Inst.at(airInst).Data;
        break;
    case InstType::SYMBOL:
        return GetSymbolType(airInst);
    case InstType::LOAD:
    case InstType::CAST:
        return m_air.Inst.at(airInst).TyOp.Ty;
    case InstType::SUB:
    case InstType::MUL:
    case InstType::DIV:
    case InstType::MOD:
    case InstType::ADD:
    case InstType::BIT_AND:
    case InstType::BIT_OR:
    case InstType::BIT_SHL:
    case InstType::BIT_SHR:
    case InstType::BIT_XOR:
        return GetAirType(m_air.Inst.at(airInst).BinOp.Lhs);
    }

    return m_mod->InternPool.GetType(poolIndex);
}

TypeInst Sema::GetSymbolTypeValue(Index airInst) {
    const auto& sym           = m_air.Inst.at(airInst).Sym;
    const symbol::Record* rec = m_mod->Map.GetRecord(sym.Decl);

    if (isNull(rec->Val)) {
        rec->Mod->GetSema(rec->KirInst)->AnalyzeBody();
    }

    if (isNull(rec->Val)) {
        return { NULL_INDEX, NULL_INDEX };
    }

    const Index val = CreateInstNoMap(InstType::LOAD, InstData::CreateTyOp(rec->Ty, airInst));

    return { rec->Ty, val };
}

Index Sema::GetSymbolType(Index airInst) {
    const auto& sym = m_air.Inst.at(airInst).Sym;
    return sym.Ty;
}

bool Sema::TryCastSameType(TypeInst& valA, TypeInst& valB) {
    if (type::areSame(valA.Ty, valB.Ty)) {
        return true;
    } else if (type::isIntType(valA.Ty)) {
        // value A is integer but the value B is not
        if (!type::isIntType(valB.Ty)) {
            return false;
        }

        // We need to find the instruction that needs to be casted
        TypeInst* value;

        if (type::canCastInt(valA.Ty, valB.Ty)) {
            valA.Ty = valB.Ty;
            value   = &valA;
        } else if (type::canCastInt(valB.Ty, valA.Ty)) {
            valB.Ty = valA.Ty;
            value   = &valB;
        }
        // values are not compatible, e.g. usize and isize
        else {
            return false;
        }

        // The value is not comptime known
        if (!IsConstant(value->Inst)) {
            value->Inst = CreateInstNoMap(InstType::CAST, InstData::CreateTyOp(value->Ty, value->Inst));
        }

        return true;
    }

    return false;
}

template <type::Operation Op> void Sema::KirArithmetic(Index inst) {
    const auto& data = m_kir.Inst.at(inst).NodePl;
    const auto& bin  = GetKirData<kir::extra::Bin>(data.Payload.Offset);

    auto lhs = GetTypeValue(bin.Lhs);
    auto rhs = GetTypeValue(bin.Rhs);

    if (isNull(lhs.Ty) || isNull(rhs.Ty)) {
        KOOLANG_ERR_MSG("LHS OR RHS HAS ERROR");
        return;
    }

    // TODO: Pointers, ...
    // if (type::isPtr(lhs.Ty)) {}

    // try cast and check if the types are the same.
    if (!TryCastSameType(lhs, rhs)) {
        KOOLANG_ERR_MSG("MISMATCHED TYPES");
        return;
    }

    // lhs / 0 or lhs % 0
    if ((Op == type::Operation::DIV || Op == type::Operation::MOD) && IsConstant(rhs.Inst)) {
        if (m_air.Inst.at(rhs.Inst).Data == pool::keys::ZERO_VALUE_INDEX) {
            KOOLANG_ERR_MSG("DIVISION BY ZERO");
            return;
        }
    }

    // TODO: Eval
    // if (AreConstants(lhs.Inst, rhs.Inst)) {
    //     EvalOp<Op>(inst, lhs, rhs);
    //     return;
    // }

    InstType type;

    switch (Op) {
    case type::Operation::ADD:
        type = InstType::ADD;
        break;
    case type::Operation::SUB:
        type = InstType::SUB;
        break;
    case type::Operation::MUL:
        type = InstType::MUL;
        break;
    case type::Operation::DIV:
        type = InstType::DIV;
        break;
    case type::Operation::MOD:
        type = InstType::MOD;
        break;
    case type::Operation::BIT_AND:
        type = InstType::BIT_AND;
        break;
    case type::Operation::BIT_OR:
        type = InstType::BIT_OR;
        break;
    case type::Operation::BIT_SHL:
        type = InstType::BIT_SHL;
        break;
    case type::Operation::BIT_SHR:
        type = InstType::BIT_SHR;
        break;
    case type::Operation::BIT_XOR:
        type = InstType::BIT_XOR;
        break;
    }

    CreateInst(type, InstData::CreateBinOp(lhs.Inst, rhs.Inst), inst);
}

template <type::Operation Op> void Sema::EvalOp(Index inst, TypeInst lhs, TypeInst rhs) {
    // TODO: floats
    KOOLANG_TODO();

    const Index lhsPoolData = m_pool.GetData(m_air.Inst.at(lhs.Inst).Data);
    const Index rhsPoolData = m_pool.GetData(m_air.Inst.at(rhs.Inst).Data);

    const auto lhsData = m_pool.GetExtra<pool::TypeValue>(lhsPoolData);
    const auto rhsData = m_pool.GetExtra<pool::TypeValue>(rhsPoolData);

    value::Result<std::uint64_t> result;
    switch (Op) {
    case type::Operation::ADD:
        if (type::isSignedInt(lhs.Ty)) {
            result = value::addSigned(m_pool.Values.at(lhsData.Val), m_pool.Values.at(rhsData.Val));
        } else {
            result = value::addUnsigned(m_pool.Values.at(lhsData.Val), m_pool.Values.at(rhsData.Val));
        }

        break;
    case type::Operation::SUB:
        if (type::isSignedInt(lhs.Ty)) {
            result = value::subSigned(m_pool.Values.at(lhsData.Val), m_pool.Values.at(rhsData.Val));
        } else {
            result = value::subUnsigned(m_pool.Values.at(lhsData.Val), m_pool.Values.at(rhsData.Val));
        }

        break;
    case type::Operation::MUL:
        if (type::isSignedInt(lhs.Ty)) {
            result = value::mulSigned(m_pool.Values.at(lhsData.Val), m_pool.Values.at(rhsData.Val));
        } else {
            result = value::mulUnsigned(m_pool.Values.at(lhsData.Val), m_pool.Values.at(rhsData.Val));
        }
        break;
    case type::Operation::DIV:
        if (type::isSignedInt(lhs.Ty)) {
            result = value::divSigned(m_pool.Values.at(lhsData.Val), m_pool.Values.at(rhsData.Val));
        } else {
            result = value::divUnsigned(m_pool.Values.at(lhsData.Val), m_pool.Values.at(rhsData.Val));
        }
        break;
    case type::Operation::MOD:
        if (type::isSignedInt(lhs.Ty)) {
            result = value::modSigned(m_pool.Values.at(lhsData.Val), m_pool.Values.at(rhsData.Val));
        } else {
            result = value::modUnsigned(m_pool.Values.at(lhsData.Val), m_pool.Values.at(rhsData.Val));
        }
        break;
    case type::Operation::BIT_AND:
        result = value::Result<std::uint64_t> { m_pool.Values.at(lhsData.Val) & m_pool.Values.at(rhsData.Val),
                                                value::ResultState::OK };
        break;
    case type::Operation::BIT_OR:
        result = value::Result<std::uint64_t> { m_pool.Values.at(lhsData.Val) | m_pool.Values.at(rhsData.Val),
                                                value::ResultState::OK };
        break;
    case type::Operation::BIT_XOR:
        result = value::Result<std::uint64_t> { m_pool.Values.at(lhsData.Val) ^ m_pool.Values.at(rhsData.Val),
                                                value::ResultState::OK };
        break;
    case type::Operation::BIT_SHL:
        if (type::isSignedInt(rhs.Ty)) {
            result = value::Result<std::uint64_t> { 0, value::ResultState::SHIFT_NEGATIVE };
        } else {
            result = value::Result<std::uint64_t> { m_pool.Values.at(lhsData.Val) << m_pool.Values.at(rhsData.Val),
                                                    value::ResultState::OK };
        };
        break;
    case type::Operation::BIT_SHR:
        result = value::Result<std::uint64_t> { m_pool.Values.at(lhsData.Val) >> m_pool.Values.at(rhsData.Val),
                                                value::ResultState::OK };
        break;
    }

    if (result.HasErr()) {
        KOOLANG_ERR_MSG("OP ERR");
        return;
    }

    if (!value::canFitInt(lhs.Ty, result.Val)) {
        KOOLANG_ERR_MSG("CANNOT FIT");
        return;
    }

    // Comptime known values 0,1
    Index resultIndex;
    if (result.Val == 0) {
        resultIndex = pool::keys::ZERO_VALUE_INDEX;
    } else if (result.Val == 1) {
        resultIndex = pool::keys::ONE_VALUE_INDEX;
    } else {
        resultIndex = m_pool.AddValue(result.Val);
    }

    const auto poolIndex = m_pool.Put(PoolKey::CreateTypeValue(lhs.Ty, resultIndex));

    CreateConstantFrom(inst, poolIndex);
}
}
