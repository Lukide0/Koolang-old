#include "Printer.h"
#include "Extra.h"
#include "util/array_util.h"

namespace kir {

Printer::Printer(const Kir& kir)
    : m_kir(kir) { }

void Printer::WriteInstRef(RefInst ref) {
    if (!ref.IsConstant()) {
        m_buffer << '%' << ref.Offset;
        return;
    }

    const auto value = ref.ToConstant();
    switch (value) {
    case RefInst::NONE:
        m_buffer << "Ref.NONE";
        break;
    case RefInst::ZERO:
        m_buffer << "Ref.ZERO";
        break;
    case RefInst::ONE:
        m_buffer << "Ref.ONE";
        break;
    case RefInst::NULL_VALUE:
        m_buffer << "Ref.NULL_VALUE";
        break;
    case RefInst::BOOL_TRUE:
        m_buffer << "Ref.BOOL_TRUE";
        break;
    case RefInst::BOOL_FALSE:
        m_buffer << "Ref.BOOL_FALSE";
        break;
    case RefInst::VOID_TYPE:
        m_buffer << "Ref.VOID_TYPE";
        break;
    case RefInst::BOOL_TYPE:
        m_buffer << "Ref.BOOL_TYPE";
        break;
    case RefInst::U8_TYPE:
        m_buffer << "Ref.U8_TYPE";
        break;
    case RefInst::I8_TYPE:
        m_buffer << "Ref.I8_TYPE";
        break;
    case RefInst::U16_TYPE:
        m_buffer << "Ref.U16_TYPE";
        break;
    case RefInst::I16_TYPE:
        m_buffer << "Ref.I16_TYPE";
        break;
    case RefInst::U32_TYPE:
        m_buffer << "Ref.U32_TYPE";
        break;
    case RefInst::I32_TYPE:
        m_buffer << "Ref.I32_TYPE";
        break;
    case RefInst::U64_TYPE:
        m_buffer << "Ref.U64_TYPE";
        break;
    case RefInst::I64_TYPE:
        m_buffer << "Ref.I64_TYPE";
        break;
    case RefInst::USIZE_TYPE:
        m_buffer << "Ref.USIZE_TYPE";
        break;
    case RefInst::ISIZE_TYPE:
        m_buffer << "Ref.ISIZE_TYPE";
        break;
    case RefInst::F16_TYPE:
        m_buffer << "Ref.F16_TYPE";
        break;
    case RefInst::F32_TYPE:
        m_buffer << "Ref.F32_TYPE";
        break;
    case RefInst::F64_TYPE:
        m_buffer << "Ref.F64_TYPE";
        break;
    case RefInst::STR_TYPE:
        m_buffer << "Ref.STR_TYPE";
        break;
    case RefInst::CHAR_TYPE:
        m_buffer << "Ref.CHAR_TYPE";
        break;
    }
}

void Printer::Write(std::string_view text) { m_buffer << text; }
void Printer::Write(char character) { m_buffer << character; }

void Printer::WriteNewLine() { m_buffer << '\n'; }

void Printer::WriteBin(std::string_view name, Index bin) {
    const auto value = m_kir.Inst.at(bin).Bin;
    Write(name);
    Write('(');
    WriteInstRef(value.Lhs);
    Write(", ");
    WriteInstRef(value.Rhs);
    Write(')');
    WriteNewLine();
}

void Printer::WriteBinInst(std::string_view name, Index nodepl) {
    const auto value = m_kir.Inst.at(nodepl).NodePl;
    const auto lhs   = m_kir.Extra.at(value.Payload.Offset);
    const auto rhs   = m_kir.Extra.at(value.Payload.Offset + 1);

    Write(name);
    Write('(');
    WriteInstRef(lhs);
    Write(", ");
    WriteInstRef(rhs);
    Write(')');
    WriteNewLine();
}

void Printer::Print() {
    if (m_kir.Inst.empty()) {
        return;
    }

    WriteInst(1);

    std::cout << m_buffer.str();
}

void Printer::WriteStr(Index str) { m_buffer << m_kir.Strings.at(str); }
void Printer::WriteCond(std::string_view text, Index extra) {
    if (!isNull(m_kir.Extra.at(extra))) {
        m_buffer << text;
    }
}

void Printer::WriteSingleOpInst(std::string_view name, Index nodepl) {
    const auto instData = m_kir.Inst.at(nodepl).NodePl;
    Write(name);
    Write('(');
    WriteInstRef(instData.Payload);
    Write(')');
    WriteNewLine();
}

void Printer::WriteInst(Index ref) {
    InstType type = m_kir.Type.at(ref);

    if (type == InstType::NONE) {
        return;
    }

    // padding
    m_buffer << std::string(m_offset * 2, ' ');

    Write('%');
    m_buffer << ref;
    Write(" = ");

    switch (type) {
    case InstType::NONE:
        KOOLANG_ERR_MSG("INVALID INSTRUCTION");
        KOOLANG_ERR_MSG("{}", ref);
        return;
    case InstType::IDENT: {
        Write("ident(");
        WriteStr(m_kir.Inst.at(ref).Ref.Offset);
        Write(')');
        WriteNewLine();
        break;
    }

    case InstType::BLOCK_INLINE:
    case InstType::BLOCK_COMPTIME_INLINE:
        if (m_kir.Type.at(ref) == InstType::BLOCK_INLINE) {
            Write("{\n");
        } else {
            Write("comptime{\n");
        }

        WriteBlockRaw(ref);
        Write('}');
        WriteNewLine();
        break;
    case InstType::BLOCK:
    case InstType::LOOP:
        WriteBlock(ref);
        WriteNewLine();
        break;
    case InstType::DECL:
        WriteDecl(ref);
        break;
    case InstType::DECL_REF:
        WriteDeclRef(ref);
        break;
    case InstType::DECL_ITEM:
        WriteDeclItem(ref);
        break;
    case InstType::DECL_TRAIT:
        WriteDeclTrait(ref);
        break;
    case InstType::DECL_IMPL:
        WriteDeclImpl(ref);
        break;
    case InstType::NAMESPACE:
        WriteNamespace(ref);
        break;
    case InstType::PARAM:
        WriteParam(ref);
        break;
    case InstType::INT: {
        const auto value = m_kir.Inst.at(ref).Int;
        Write("int(");
        m_buffer << value;
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::FLOAT: {
        const auto value = m_kir.Inst.at(ref).Float;
        Write("float(");
        m_buffer << value;
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::STR: {
        const auto value = m_kir.Inst.at(ref).StrTok;
        Write("str(");
        WriteStr(value.Str);
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::CHAR: {
        const auto value = m_kir.Inst.at(ref).StrTok;
        Write("char(");
        WriteStr(value.Str);
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::AS:
        WriteBin("as", ref);
        break;
    case InstType::ADD:
        WriteBinInst("add", ref);
        break;
    case InstType::SUB:
        WriteBinInst("sub", ref);
        break;
    case InstType::MUL:
        WriteBinInst("mul", ref);
        break;
    case InstType::DIV:
        WriteBinInst("div", ref);
        break;
    case InstType::MOD:
        WriteBinInst("mod", ref);
        break;
    case InstType::ARR_EL:
        WriteBinInst("arr_el", ref);
        break;
    case InstType::FIELD: {
        const auto value              = m_kir.Inst.at(ref).NodePl;
        const extra::FieldExpr& field = deserializeFromVec<extra::FieldExpr>(m_kir.Extra, value.Payload.Offset);
        Write("field(");

        WriteInstRef(field.Base);
        Write(", ");
        WriteInstRef(field.Field);
        Write(")");
        WriteNewLine();
        break;
    }
    case InstType::CMP_LS:
        WriteBinInst("cmp_ls", ref);
        break;
    case InstType::CMP_GT:
        WriteBinInst("cmp_gt", ref);
        break;
    case InstType::CMP_LSE:
        WriteBinInst("cmp_lse", ref);
        break;
    case InstType::CMP_GTE:
        WriteBinInst("cmp_gte", ref);
        break;
    case InstType::CMP_EQ:
        WriteBinInst("cmp_eq", ref);
        break;
    case InstType::CMP_NEQ:
        WriteBinInst("cmp_neq", ref);
        break;
    case InstType::BIT_AND:
        WriteBinInst("bit_and", ref);
        break;
    case InstType::BIT_OR:
        WriteBinInst("bit_or", ref);
        break;
    case InstType::BIT_SHL:
        WriteBinInst("bit_shl", ref);
        break;
    case InstType::BIT_SHR:
        WriteBinInst("bit_shr", ref);
        break;
    case InstType::BIT_XOR:
        WriteBinInst("bit_xor", ref);
        break;
    case InstType::BREAK_INLINE:
        WriteBin("break_inline", ref);
        break;
    case InstType::ARR_SHORT_INIT:
        WriteBinInst("arr_short_init", ref);
        break;
    case InstType::ARR_INIT: {
        const auto& instValue = m_kir.Inst.at(ref).NodePl.Payload.Offset;
        Write("arr_init({");
        WriteRange(instValue + 1, m_kir.Extra.at(instValue));
        Write("})");
        WriteNewLine();
        break;
    }
    case InstType::TUPLE: {
        const auto& instValue = m_kir.Inst.at(ref).NodePl.Payload.Offset;
        Write("tuple({");
        WriteRange(instValue + 1, m_kir.Extra.at(instValue));
        Write("})");
        WriteNewLine();
        break;
    }
    case InstType::CAST:
        WriteBin("cast", ref);
        break;
    case InstType::BOOL_NEG:
        WriteSingleOpInst("bool_neg", ref);
        break;
    case InstType::BIT_NEG:
        WriteSingleOpInst("bit_neg", ref);
        break;
    case InstType::GET_ADDR:
        WriteSingleOpInst("get_addr", ref);
        break;
    case InstType::DEREF:
        WriteSingleOpInst("deref", ref);
        break;
    case InstType::INT_NEG:
        WriteSingleOpInst("int_neg", ref);
        break;
    case InstType::UNWRAP:
        WriteSingleOpInst("unwrap", ref);
        break;
    case InstType::SLICE_FULL: {
        const auto& instData = m_kir.Inst.at(ref).NodePl.Payload.Offset;
        Write("slice_full(");
        WriteInstRef(m_kir.Extra.at(instData));
        Write(", {");
        WriteInstRef(m_kir.Extra.at(instData + 1));
        Write(", ");
        WriteInstRef(m_kir.Extra.at(instData + 2));
        Write("})");
        WriteNewLine();
        break;
    }
    case InstType::SLICE_START:
        WriteBin("slice_start", ref);
        break;
    case InstType::SLICE_END:
        WriteBin("slice_end", ref);
        break;
    case InstType::CALL: {
        const auto& instData = m_kir.Inst.at(ref).NodePl.Payload.Offset;
        Write("call(");
        WriteInstRef(m_kir.Extra.at(instData));
        Write(", {");
        WriteRange(instData + 2, m_kir.Extra.at(instData + 1));
        Write("})");
        WriteNewLine();
        break;
    }
    case InstType::ARRAY_TYPE:
        WriteBinInst("array_type", ref);
        break;
    case InstType::SLICE_TYPE: {

        const auto& instData = m_kir.Inst.at(ref).NodePl.Payload.Offset;
        Write("slice_type(");
        WriteInstRef(instData);
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::PTR_TYPE: {
        const auto& instData = m_kir.Inst.at(ref).NodePl.Payload.Offset;
        Write("ptr_type(");
        m_buffer << m_kir.Extra.at(instData);
        Write(", ");
        WriteInstRef(m_kir.Extra.at(instData + 1));
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::TUPLE_TYPE: {
        const auto& instData = m_kir.Inst.at(ref).NodePl.Payload.Offset;
        Write("tuple_type({");
        WriteRange(instData + 1, m_kir.Extra.at(instData));
        Write("})");
        WriteNewLine();
        break;
    }
    case InstType::DYN_TYPE: {
        const auto& instData = m_kir.Inst.at(ref).NodePl.Payload.Offset;
        Write("dyn_type({");
        WriteRange(instData + 1, m_kir.Extra.at(instData));
        Write("})");
        WriteNewLine();
        break;
    }
    case InstType::REF_TYPE: {
        const auto& instData = m_kir.Inst.at(ref).NodePl;
        Write("ref_type(");
        WriteInstRef(instData.Payload);
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::STRUCT_INIT_EMPTY: {
        const auto& instData = m_kir.Inst.at(ref).NodePl;
        Write("struct_init_empty(");
        WriteInstRef(instData.Payload);
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::STRUCT_INIT:
        WriteStructInit(ref);
        break;
    case InstType::LOGIC_AND: {
        const auto& instData = m_kir.Inst.at(ref).NodePl.Payload.Offset;
        Write("logic_and(");
        WriteInstRef(m_kir.Extra.at(instData));
        Write(", ");
        WriteBlockInline(m_kir.Extra.at(instData + 1));
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::LOGIC_OR: {
        const auto& instData = m_kir.Inst.at(ref).NodePl.Payload.Offset;
        Write("logic_or(");
        WriteInstRef(m_kir.Extra.at(instData));
        Write(", ");
        WriteBlockInline(m_kir.Extra.at(instData + 1));
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::STORE_NODE:
        WriteBinInst("store_node", ref);
        break;

    case InstType::STORE:
        WriteBin("store", ref);
        break;
    case InstType::STORE_INFERRED:
        WriteBin("store_inferred", ref);
        break;
    case InstType::ALLOC: {
        const auto instData = m_kir.Inst.at(ref).NodePl.Payload;
        Write("alloc(");
        WriteInstRef(instData);
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::ALLOC_MUT: {
        const auto instData = m_kir.Inst.at(ref).NodePl.Payload;
        Write("alloc_mut(");
        WriteInstRef(instData);
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::ALLOC_INFERRED:
        Write("alloc_mut_inferred()");
        WriteNewLine();
        break;
    case InstType::ALLOC_MUT_INFERRED:
        Write("alloc_inferred()");
        WriteNewLine();
        break;

    case InstType::FIELD_SHORT: {
        const auto& instData = m_kir.Inst.at(ref).Bin;
        Write("field_short(");
        WriteInstRef(instData.Lhs);
        Write(", ");
        Write(std::to_string(instData.Rhs.Offset));
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::LOAD:
        Write("load(");
        WriteInstRef(m_kir.Inst.at(ref).Ref);
        Write(')');
        WriteNewLine();
        break;

    case InstType::DISCARD_DESTRUCTOR:
        Write("discard_destructor(");
        WriteInstRef(m_kir.Inst.at(ref).Ref);
        Write(')');
        WriteNewLine();
        break;
    case InstType::CONDBR:
        WriteIf(ref);
        break;
    case InstType::GOTO:
        Write("goto(");
        WriteInstRef(m_kir.Inst.at(ref).Ref);
        Write(')');
        WriteNewLine();
        break;
    case InstType::REPEAT:
        Write("repeat(");
        WriteInstRef(m_kir.Inst.at(ref).Ref);
        Write(')');
        WriteNewLine();
        break;

    case InstType::INDEXABLE_LEN:
        Write("indexable_len(");
        WriteInstRef(m_kir.Inst.at(ref).Ref);
        Write(')');
        WriteNewLine();
        break;
    case InstType::BREAK: {
        const auto& instData = m_kir.Inst.at(ref).NodePl;
        Write("break(");
        if (instData.Payload.Offset != RefInst(RefInst::NONE).Offset) {
            WriteInstRef(instData.Payload.Offset);
        }
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::RETURN: {
        const auto& instData = m_kir.Inst.at(ref).NodePl;
        Write("return(");
        if (instData.Payload.Offset != RefInst(RefInst::NONE).Offset) {
            WriteInstRef(instData.Payload);
        }
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::CONTINUE: {
        const auto& instData = m_kir.Inst.at(ref).NodePl;
        Write("continue(");
        if (instData.Payload.Offset != RefInst(RefInst::NONE).Offset) {
            WriteStr(instData.Payload.Offset);
        }
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::DECL_FN:
        WriteDeclFn(ref);
        break;
    case InstType::DECL_ENUM: {
        const auto& instData = m_kir.Inst.at(ref).Bin;
        const auto enumData  = deserializeFromVec<extra::DeclEnum>(m_kir.Extra, instData.Lhs.Offset);

        Write("delc_enum(\"");
        WriteCond("pub ", enumData.DeclInfo.Vis);
        WriteStr(enumData.DeclInfo.Name);
        Write("\", ");
        WriteInstRef(enumData.Type);
        Write(", ");
        WriteBlockInline(instData.Rhs.Offset);
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::DECL_STRUCT: {
        const auto& instData  = m_kir.Inst.at(ref).Bin;
        const auto structData = deserializeFromVec<extra::DeclStruct>(m_kir.Extra, instData.Lhs.Offset);

        Write("delc_struct(\"");
        WriteCond("pub ", structData.DeclInfo.Vis);
        WriteStr(structData.DeclInfo.Name);
        Write("\", ");
        WriteBlockInline(instData.Rhs.Offset);
        Write(')');
        WriteNewLine();
        break;
    }

    case InstType::DECL_VARIANT: {
        const auto& instData   = m_kir.Inst.at(ref).Bin;
        const auto variantData = deserializeFromVec<extra::DeclVariant>(m_kir.Extra, instData.Lhs.Offset);

        Write("delc_variant(\"");
        WriteCond("pub ", variantData.DeclInfo.Vis);
        WriteStr(variantData.DeclInfo.Name);
        Write("\", ");
        WriteBlockInline(instData.Rhs.Offset);
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::ENUM_FIELD: {
        const auto& instData = m_kir.Inst.at(ref).NodePl;
        const auto data      = deserializeFromVec<extra::DeclEnumField>(m_kir.Extra, instData.Payload.Offset);
        Write("enum_field(\"");
        WriteStr(data.Name);
        Write("\", ");
        WriteInstRef(data.Value);
        Write(')');
        WriteNewLine();
        break;
    }
    case InstType::STRUCT_FIELD: {
        const auto& instData = m_kir.Inst.at(ref).NodePl;
        const auto fieldData = deserializeFromVec<extra::DeclStructField>(m_kir.Extra, instData.Payload.Offset);

        Write("struct_field(\"");
        WriteCond("pub ", fieldData.DeclInfo.Vis);
        WriteStr(fieldData.DeclInfo.Name);
        Write("\", ");
        WriteInstRef(fieldData.Type);
        Write(", ");
        WriteInstRef(fieldData.DefaultValue);
        Write(')');
        WriteNewLine();
        break;
        break;
    }
    }
}

void Printer::WriteBlockRaw(Index block) {
    m_offset += 1;

    const auto& instData = m_kir.Inst.at(block).NodePl;
    const Index meta     = instData.Payload.Offset;
    const Index count    = m_kir.Extra.at(meta);

    for (Index i = 0; i < count; i++) {
        WriteInst(m_kir.Extra.at(meta + 1 + i));
    }

    m_offset -= 1;
    Write(std::string(m_offset * 2, ' '));
}

void Printer::WriteBlock(Index block) {
    if (isNull(block)) {
        Write("block{}");
        return;
    } else if (m_kir.Type.at(block) == InstType::LOOP) {
        Write("loop{\n");
    } else {
        Write("block{\n");
    }
    WriteBlockRaw(block);
    Write('}');
}

void Printer::WriteBlockInline(Index block) {
    if (isNull(block)) {
        Write("{}");
        return;
    }

    m_buffer << '%' << block << " = ";

    if (m_kir.Type.at(block) == InstType::BLOCK_INLINE) {
        Write("{\n");
    } else {
        Write("comptime{\n");
    }

    WriteBlockRaw(block);
    Write("}");
}

void Printer::WriteRange(Index start, Index count) {
    if (count == 0) {
        return;
    }

    WriteInstRef(m_kir.Extra.at(start));

    for (Index i = 1; i < count; i++) {
        Write(", ");
        WriteInstRef(m_kir.Extra.at(start + i));
    }
}

void Printer::WriteDecl(Index ref) {
    const auto& instData  = m_kir.Inst.at(ref).Bin;
    const Index visIndex  = instData.Lhs.Offset;
    const Index nameIndex = m_kir.Extra.at(instData.Lhs.Offset + 2);

    Write("decl(\"");
    WriteCond("pub ", visIndex);
    WriteStr(nameIndex);
    Write("\", ");
    WriteBlockInline(instData.Rhs.Offset);
    Write(')');
    WriteNewLine();
}

void Printer::WriteDeclFn(Index ref) {
    const auto& instData    = m_kir.Inst.at(ref).Bin;
    const extra::DeclFn& fn = deserializeFromVec<extra::DeclFn>(m_kir.Extra, instData.Lhs.Offset);

    Write("decl_fn(\"");
    WriteCond("pub ", fn.DeclInfo.Vis);
    WriteCond("const ", fn.Modifiers & 1);
    WriteStr(fn.DeclInfo.Name);
    Write("\", ");
    WriteBlockInline(fn.RetTypeInst);
    Write(", ");
    WriteBlockInline(fn.Params);
    Write(", ");

    if (!isNull(instData.Rhs.Offset)) {
        m_buffer << '%' << instData.Rhs.Offset << " = ";
    }

    WriteBlock(instData.Rhs.Offset);
    Write(')');
    WriteNewLine();
}

void Printer::WriteDeclTrait(Index ref) {
    const auto& instData = m_kir.Inst.at(ref).Bin;
    const auto& trait    = deserializeFromVec<extra::DeclTrait>(m_kir.Extra, instData.Lhs.Offset);

    Write("decl_trait(\"");
    WriteCond("pub ", trait.DeclInfo.Vis);
    WriteStr(trait.DeclInfo.Name);
    Write("\", ");
    WriteBlockInline(instData.Rhs.Offset);
    Write(')');
    WriteNewLine();
}

void Printer::WriteDeclImpl(Index ref) {
    const auto& instData = m_kir.Inst.at(ref).Bin;
    const auto& impl     = deserializeFromVec<extra::DeclImpl>(m_kir.Extra, instData.Lhs.Offset);

    Write("decl_impl(\"");
    WriteCond("pub", impl.DeclInfo.Vis);
    Write("\", ");
    WriteInstRef(impl.StructPath);
    Write(", ");
    WriteInstRef(impl.TraitPath);
    Write(", ");
    WriteBlockInline(instData.Rhs.Offset);
    Write(')');
    WriteNewLine();
}

void Printer::WriteDeclRef(Index ref) {
    const auto& instData  = m_kir.Inst.at(ref).TokPl;
    const Index nameIndex = instData.Payload.Offset;

    Write("decl_ref(\"");
    WriteStr(nameIndex);
    Write("\")");
    WriteNewLine();
}

void Printer::WriteDeclItem(Index ref) {
    const auto& instData = m_kir.Inst.at(ref).NodePl.Payload.Offset;
    const auto data      = deserializeFromVec<extra::DeclItem>(m_kir.Extra, instData);

    Write("decl_item(\"");
    WriteStr(data.Name);
    Write("\")");
    WriteNewLine();
}

void Printer::WriteNamespace(Index ref) {
    const auto& instData       = m_kir.Inst.at(ref).NodePl.Payload.Offset;
    const Index count          = m_kir.Extra.at(instData);
    const Index firstNameIndex = instData + 1;

    Write("namespace(\"");

    WriteStr(m_kir.Extra.at(firstNameIndex));
    for (Index i = 1; i < count; i++) {
        Write("::");
        WriteStr(m_kir.Extra.at(firstNameIndex + i));
    }
    Write("\")");
    WriteNewLine();
}

void Printer::WriteStructInit(Index ref) {
    const auto& instData = m_kir.Inst.at(ref).NodePl.Payload.Offset;

    const auto structInit = deserializeFromVec<extra::StructInit>(m_kir.Extra, instData);

    const Index fieldsStart = instData + sizeof(structInit) / sizeof(Index);

    Write("struct_init(");
    WriteInstRef(structInit.PathInst);
    Write(", fields={\n");

    std::string offset((m_offset + 1) * 2, ' ');

    for (Index i = 0; i < structInit.FieldsCount; i++) {
        m_buffer << offset;
        WriteStr(m_kir.Extra.at(fieldsStart + i * 2));
        Write('=');
        WriteInstRef(m_kir.Extra.at(fieldsStart + 1 + i * 2));
        WriteNewLine();
    }

    m_buffer << std::string(m_offset * 2, ' ');
    Write("})");
    WriteNewLine();
}

void Printer::WriteFunc(Index ref) {
    const auto& instData = m_kir.Inst.at(ref).NodePl;

    Write("func{");
    WriteBlockInline(instData.Payload.Offset);
    Write('}');
    WriteNewLine();
}

void Printer::WriteParam(Index ref) {
    const auto& instData = m_kir.Inst.at(ref).NodePl;
    const auto data      = deserializeFromVec<extra::Param>(m_kir.Extra, instData.Payload.Offset);

    Write("param(\"");
    WriteStr(data.Name);
    Write("\", ");
    WriteInstRef(data.Type);
    Write(')');
    WriteNewLine();
}

void Printer::WriteIf(Index ref) {
    const auto& instData = m_kir.Inst.at(ref).Bin;
    Write("condbr(");
    WriteInstRef(instData.Lhs);
    Write(", body_len=");
    m_buffer << m_kir.Extra.at(instData.Rhs.Offset);
    Write(", end=");
    WriteInstRef(m_kir.Extra.at(instData.Rhs.Offset + 1));
    Write(')');
    WriteNewLine();
}

}
