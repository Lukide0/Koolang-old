#ifndef KOOLANG_KIR_ASTGEN_H
#define KOOLANG_KIR_ASTGEN_H

#include "Inst.h"
#include "InstData.h"
#include "RefInst.h"
#include "Scope.h"
#include "ast/Ast.h"
#include "util/Cache.h"
#include "util/ConstMap.h"
#include "util/Index.h"
#include "util/StrCache.h"
#include "util/array_util.h"
#include <cstddef>
#include <initializer_list>

namespace kir {

class AstGen {
public:
    AstGen(Kir& kir, const ast::Ast& tree);

    void Generate();

private:
    static constexpr Index GLOBAL_SCOPE_INDEX = 1;

    struct Label;

    Kir& m_kir;
    const ast::Ast& m_tree;

    std::vector<Scope> m_scopes;
    std::vector<SymbolMeta> m_symbolMeta;
    std::vector<Label> m_labels;

    StrCache m_strings;
    Cache m_cache;
    Index m_prevSymbol = NULL_INDEX;
    Index m_currScope;

    struct Label {
        Index Inst;
        Index Str;
    };

    struct InstCache {
        Index Inst;
        Index Cache;
    };

    // STATEMENTS ///////////////////////////////////////////////////////////

    void GenGlobConst(Index node);
    void GenVar(Index node);
    void GenConst(Index node);
    void GenStatic(Index node);

    void GenFn(Index node);
    void GenVariant(Index node);
    void GenEnum(Index node);
    void GenStruct(Index node);
    void GenPattern(Index node, RefInst value);

    void GenRawBlock(Index node);

    void GenImpl(Index node);
    void GenTrait(Index node);

    void GenFlow(Index node);
    void GenFor(Index node);
    void GenWhile(Index node);

    // Returns index of the instruction.
    void GenIf(Index node);
    RefInst GenAs(RefInst expression, RefInst type);

    void GenDiscard(Index node);

    // EXPRESSIONS ////////////////////////////////////////////////////////////

    RefInst GenExpr(Index node);
    RefInst GenType(Index node);

    RefInst GenPath(Index node);
    RefInst GenLiteral(Index node);
    RefInst GenArray(Index node);
    RefInst GenArrayShort(Index node);
    RefInst GenTuple(Index node);
    RefInst GenCast(Index node);
    RefInst GenClosure(Index node);
    RefInst GenSingleOp(Index node);
    RefInst GenUnwrap(Index node);
    RefInst GenBinOp(Index node);
    RefInst GenSlice(Index node);
    RefInst GenCall(Index node);
    RefInst GenStructExpr(Index node);
    RefInst GenField(Index rhsNode, RefInst lhs);

    // UTIL ///////////////////////////////////////////////////////////////////
    template <typename T> Index CreateExtraFrom(const T& value);
    template <std::size_t N> Index CreateExtraFrom(std::array<Index, N>&& items);
    Index CreateExtraFromCache(Index start);
    Index CreateExtra(Index item);

    void AddToCache(Index value);

    const ast::Node& GetNode(Index node) const;
    Index GetNodeToken(Index node) const;
    Index GetNodeMeta(Index metaIndex) const;
    ast::Tag GetNodeTag(Index node) const;

    RefInst GetOrCreateStr(std::string_view str);
    RefInst GetOrCreateStr(Index token);

    // INSTRUCTIONS METHODS ///////////////////////////////////////////////////
    Index PrepareInst();
    InstCache PrepareInstWithCache();

    void SetInst(Index inst, InstType type, InstData data);
    Index CreateInst(InstType type, InstData data);

    Index GetExtraSize() const;
    void AddToExtra(Index value) const;
    Index ReserveExtra(Index size);
    void SetExtra(Index index, Index value);

    InstCache EnterBlock();
    void CreateBlock(InstType type, InstCache block, Index node, RefInst returnValue = NULL_INDEX);
    // SYMBOLS ////////////////////////////////////////////////////////////////

    Index CreateOrGetScope(Scope::Type type, Index name = NULL_INDEX, Index meta = NULL_INDEX);
    Index CreateOrGetScopeCustom(Scope::Type type, Index parent, Index name = NULL_INDEX, Index meta = NULL_INDEX);
    Index CreateSymbolMeta(Index inst = NULL_INDEX, Index flags = NULL_INDEX);

    void CreateSymbol(RefInst identifier, Index decl, Index scope, Index flags = NULL_INDEX);

    Index GetSymbolInScope(Index name, Index scope);

    void EnterScope(Scope::Type type, Index name = NULL_INDEX, Index meta = NULL_INDEX);
    void ExitScope();

    // LABELS /////////////////////////////////////////////////////////////////
    void AddLabel(Index str, Index inst);
    void PopLabel();
    Index FindLabel(Index str);
};

// IMPLEMENTATION /////////////////////////////////////////////////////////////

template <typename T> inline Index AstGen::CreateExtraFrom(const T& value) {
    const auto index = static_cast<Index>(m_kir.Extra.size());
    serializeToVec(m_kir.Extra, value);
    return index;
}

inline Index AstGen::CreateExtra(Index item) {
    m_kir.Extra.push_back(item);
    return static_cast<Index>(m_kir.Extra.size() - 1);
}

template <std::size_t N> inline Index AstGen::CreateExtraFrom(std::array<Index, N>&& items) {
    static_assert(!items.empty());

    const auto index = static_cast<Index>(m_kir.Extra.size());
    m_kir.Extra.insert(m_kir.Extra.end(), items.begin(), items.end());
    return index;
}

inline Index AstGen::CreateExtraFromCache(Index start) { return m_cache.InsertInto(m_kir.Extra, start); }

inline Index AstGen::GetExtraSize() const { return static_cast<Index>(m_kir.Extra.size()); }

inline void AstGen::AddToExtra(Index value) const { m_kir.Extra.push_back(value); }

inline const ast::Node& AstGen::GetNode(Index node) const { return m_tree.Nodes.at(node); }

inline Index AstGen::GetNodeToken(Index node) const { return m_tree.NodeTokens.at(node); }

inline Index AstGen::GetNodeMeta(Index metaIndex) const { return m_tree.Meta.at(metaIndex); }

inline ast::Tag AstGen::GetNodeTag(Index node) const { return m_tree.NodeTags.at(node); }

inline void AstGen::AddToCache(Index value) { m_cache.Add(value); }

inline Index AstGen::ReserveExtra(Index size) {
    const auto extraSize = GetExtraSize();
    m_kir.Extra.resize(extraSize + size);
    return extraSize;
}

inline void AstGen::SetExtra(Index index, Index value) { m_kir.Extra[index] = value; }

inline Index AstGen::PrepareInst() {
    const auto index = static_cast<Index>(m_kir.Inst.size());

    m_kir.Inst.emplace_back();
    m_kir.Type.emplace_back();

    return index;
}

inline AstGen::InstCache AstGen::PrepareInstWithCache() {
    const auto index = PrepareInst();
    AddToCache(index);
    return InstCache { index, m_cache.Size() };
}

inline void AstGen::SetInst(Index inst, InstType type, InstData data) {
    m_kir.Inst[inst] = data;
    m_kir.Type[inst] = type;
}

inline Index AstGen::CreateInst(InstType type, InstData data) {
    const auto index = static_cast<Index>(m_kir.Inst.size());

    m_kir.Inst.push_back(data);
    m_kir.Type.push_back(type);

    AddToCache(index);

    return index;
}

inline AstGen::InstCache AstGen::EnterBlock() {
    const Index inst  = PrepareInst();
    const Index cache = m_cache.Size();

    // block size
    AddToCache(0);

    return InstCache { inst, cache };
}

inline void AstGen::CreateBlock(InstType type, InstCache block, Index node, RefInst returnValue) {
    const Index instCount = m_cache.Size() - block.Cache - static_cast<Index>(isNull(returnValue.Offset));
    m_cache.Set(block.Cache, instCount);

    if (!isNull(returnValue.Offset)) {
        DISCARD_VALUE(CreateInst(InstType::BREAK_INLINE, InstData::CreateBin(block.Inst, returnValue)));
    }

    const Index extra = CreateExtraFromCache(block.Cache);
    SetInst(block.Inst, type, InstData::CreateNodePl(node, extra));
}

inline RefInst AstGen::GetOrCreateStr(std::string_view str) {
    using namespace std::literals::string_view_literals;

    static constexpr auto Primitives = std::to_array<std::pair<std::string_view, RefInst::Constants>>({
        { "null"sv, RefInst::NULL_VALUE }, { "false"sv, RefInst::BOOL_FALSE }, { "true"sv, RefInst::BOOL_TRUE },
        { "void"sv, RefInst::VOID_TYPE },  { "bool"sv, RefInst::BOOL_TYPE },   { "u8"sv, RefInst::U8_TYPE },
        { "i8"sv, RefInst::I8_TYPE },      { "u16"sv, RefInst::U16_TYPE },     { "i16"sv, RefInst::I16_TYPE },
        { "u32"sv, RefInst::U32_TYPE },    { "i32"sv, RefInst::I32_TYPE },     { "u64"sv, RefInst::U64_TYPE },
        { "i64"sv, RefInst::I64_TYPE },    { "usize"sv, RefInst::USIZE_TYPE }, { "isize"sv, RefInst::ISIZE_TYPE },
        { "f16"sv, RefInst::F16_TYPE },    { "f32"sv, RefInst::F32_TYPE },     { "f64"sv, RefInst::F64_TYPE },
        { "str"sv, RefInst::STR_TYPE },    { "char"sv, RefInst::CHAR_TYPE },
    });

    static constexpr auto keywords = ConstMap<std::string_view, RefInst::Constants, Primitives.size()> { Primitives };

    const auto value = keywords.get(str, RefInst::NONE);
    if (value != RefInst::NONE) {
        return { value };
    } else {
        return m_strings.GetOrCreateStr(str);
    }
}

inline RefInst AstGen::GetOrCreateStr(Index token) { return GetOrCreateStr(m_tree.Tokens.GetTokContent(token)); }

inline Index AstGen::CreateOrGetScope(Scope::Type type, Index name, Index meta) {
    return CreateOrGetScopeCustom(type, m_currScope, name, meta);
}

inline Index AstGen::CreateSymbolMeta(Index inst, Index flags) {
    const auto meta = static_cast<Index>(m_symbolMeta.size());
    m_symbolMeta.emplace_back(inst, flags);
    return meta;
}

}

#endif
