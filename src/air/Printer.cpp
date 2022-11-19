#include "Printer.h"
#include "Module.h"
#include "air/symbol/Record.h"
#include "air/type.h"

namespace air {

std::string simpleTypeToStr(pool::SimpleType type);
std::string simpleValueToStr(pool::SimpleValue value);

Printer::Printer(const Sema* sema)
    : m_sema(sema)
    , m_air(sema->GetAir()) { }

void Printer::Print() {
    const symbol::Record* rec = m_sema->GetRecord();

    m_buffer << "NAME: " << rec->Name << '\n';

    for (Index i = 1; i < m_air.Inst.size(); i++) {
        WriteInst(i);
    }

    std::cout << m_buffer.str();
}

void Printer::WriteInst(Index ref) {
    const auto tag = m_air.Type.at(ref);

    m_buffer << '%' << ref << " = ";

    switch (tag) {
    case InstType::CONSTANT:
        WriteConstant(ref);
        break;
    case InstType::SYMBOL:
        WriteSymbol(ref);
        break;
    case InstType::LOAD: {
        const auto& data = m_air.Inst.at(ref).TyOp;
        m_buffer << "load(";
        WriteInternPoolData(data.Ty);
        m_buffer << ", %" << data.Operand << ')';
        break;
    }
    case InstType::CAST: {
        const auto& data = m_air.Inst.at(ref).TyOp;
        m_buffer << "cast(";
        WriteInternPoolData(data.Ty);
        m_buffer << ", %" << data.Operand << ')';
        break;
    }
    case InstType::ADD:
        WriteBinOp(ref, "add");
        break;
    case InstType::SUB:
        WriteBinOp(ref, "sub");
        break;
    case InstType::MUL:
        WriteBinOp(ref, "mul");
        break;
    case InstType::DIV:
        WriteBinOp(ref, "div");
        break;
    case InstType::MOD:
        WriteBinOp(ref, "mod");
        break;
    case InstType::BIT_AND:
        WriteBinOp(ref, "bit_and");
        break;
    case InstType::BIT_OR:
        WriteBinOp(ref, "bit_or");
        break;
    case InstType::BIT_SHL:
        WriteBinOp(ref, "bit_shl");
        break;
    case InstType::BIT_SHR:
        WriteBinOp(ref, "bit_shr");
        break;
    case InstType::BIT_XOR:
        WriteBinOp(ref, "bit_xor");
        break;
    }

    m_buffer << '\n';

    DISCARD_VALUE(ref);
}

void Printer::WriteBinOp(Index inst, std::string_view name) {
    const auto& data = m_air.Inst.at(inst).BinOp;
    m_buffer << name << "(%" << data.Lhs << ", %" << data.Rhs << ')';
}

void Printer::WriteConstant(Index inst) {
    const Index poolIndex = m_air.Inst.at(inst).Data;
    m_buffer << "constant(";
    WriteInternPoolData(poolIndex);
    m_buffer << ')';
}

void Printer::WriteSymbol(Index inst) {
    const auto& sym              = m_air.Inst.at(inst).Sym;
    const symbol::Record* record = m_sema->GetModule()->Map.GetRecord(sym.Decl);

    m_buffer << "symbol(\"" << record->Name << "\", ";
    WriteInternPoolData(sym.Ty);
    m_buffer << ')';
}

void Printer::WriteInternPoolData(Index index) {
    if (Pool::IsKnownKey(index)) {
        WriteKnownInternPoolKey(index);
        return;
    }

    const Pool& pool  = m_sema->GetModule()->InternPool;
    const auto keyRef = pool.GetKeyRef(index);

    switch (keyRef.Tag) {
    case pool::KeyTag::BYTES:
    case pool::KeyTag::ARR_TYPE:
        m_buffer << "TODO";
        break;
    case pool::KeyTag::TYPE_VALUE:
        WritePoolTypeValue(pool, keyRef.Extra);
        break;

    case pool::KeyTag::INT:
        WritePoolInt(pool, keyRef.Extra);
        break;
    default:
        break;
    }
}

std::string simpleTypeToStr(pool::SimpleType type) {
    switch (type) {
    case pool::SimpleType::VOID:
        return "void";
    case pool::SimpleType::BOOL:
        return "bool";
    case pool::SimpleType::U8:
        return "u8";
    case pool::SimpleType::I8:
        return "i8";
    case pool::SimpleType::U16:
        return "u16";
    case pool::SimpleType::I16:
        return "i16";
    case pool::SimpleType::U32:
        return "u32";
    case pool::SimpleType::I32:
        return "i32";
    case pool::SimpleType::U64:
        return "u64";
    case pool::SimpleType::I64:
        return "i64";
    case pool::SimpleType::USIZE:
        return "usize";
    case pool::SimpleType::ISIZE:
        return "isize";
    case pool::SimpleType::F16:
        return "f16";
    case pool::SimpleType::F32:
        return "f32";
    case pool::SimpleType::F64:
        return "f64";
    case pool::SimpleType::COMPTIME_INT:
        return "comptime_int";
    case pool::SimpleType::COMPTIME_FLOAT:
        return "comptime_float";
    case pool::SimpleType::CHAR:
        return "char";
    case pool::SimpleType::STR:
        return "str";
    }

    return "";
}

std::string simpleValueToStr(pool::SimpleValue value) {
    switch (value) {
    case pool::SimpleValue::ZERO:
        return "0";
    case pool::SimpleValue::ONE:
        return "1";
    case pool::SimpleValue::NULL_PTR:
        return "nullptr";
    case pool::SimpleValue::BOOL_TRUE:
        return "true";
    case pool::SimpleValue::BOOL_FALSE:
        return "false";
    }

    return "";
}
void Printer::WritePoolTypeValue(const Pool& pool, Index extra) {
    const auto intData = pool.GetExtra<pool::TypeValue>(extra);

    WriteInternPoolData(intData.Ty);
    m_buffer << ", ";

    if (type::isFloat(intData.Ty)) {
        m_buffer << reinterpret_cast<const double&>(pool.Values.at(intData.Val));
    } else if (type::isSignedInt(intData.Ty)) {
        m_buffer << static_cast<std::int64_t>(pool.Values.at(intData.Val));
    } else {
        m_buffer << pool.Values.at(intData.Val);
    }
}

void Printer::WritePoolInt(const Pool& pool, Index extra) {
    const auto intData = pool.GetExtra<pool::Int>(extra);

    WriteInternPoolData(intData.Ty);
    m_buffer << ", ";

    if (type::isUnsignedInt(intData.Ty)) {
        m_buffer << pool.Values.at(intData.ValueIndex);
    } else {
        m_buffer << static_cast<std::int64_t>(pool.Values.at(intData.ValueIndex));
    }
}

void Printer::WriteKnownInternPoolKey(Index index) {
    const auto& key = pool::keys::ALL_KEYS.at(index);

    switch (key.Tag) {
    case pool::KeyTag::NONE:
        m_buffer << "Ref.None";
        break;
    case pool::KeyTag::SIMPLE_TYPE:
        m_buffer << simpleTypeToStr(key.Value.SimpleTy);
        break;
    case pool::KeyTag::SIMPLE_VALUE:
        m_buffer << simpleValueToStr(key.Value.SimpleVal);
        break;
    default:
        KOOLANG_UNREACHABLE();
    }
}

}
