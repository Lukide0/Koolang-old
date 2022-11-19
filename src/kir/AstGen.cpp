#include "AstGen.h"
#include "Extra.h"
#include "util/convert.h"
#include <ranges>

namespace kir {

std::string pathNodeToFilePath(Index node, const ast::Ast& tree) {
    const ast::Node& pathNode = tree.Nodes.at(node);

    std::string path(tree.Tokens.GetTokContent(pathNode.Lhs));
    for (Index i = pathNode.Lhs + 2; i <= pathNode.Rhs; i += 2) {
        path += '/';
        path += tree.Tokens.GetTokContent(i);
    }

    return path;
}

AstGen::AstGen(Kir& kir, const ast::Ast& tree)
    : m_kir(kir)
    , m_tree(tree)
    , m_strings(m_kir.Strings) {
    kir.Inst.reserve(tree.Nodes.size());
    kir.Type.reserve(tree.NodeTags.size());
    kir.Extra.reserve(tree.Nodes.size());

    kir.Imports.reserve(tree.Imports.size());

    // reserve 0-index
    kir.Inst.emplace_back();
    kir.Type.emplace_back();

    // NULL SCOPE
    m_scopes.emplace_back(Scope::TOP, NULL_INDEX);
    // GLOBAL SCOPE
    m_scopes.emplace_back(Scope::TOP, NULL_INDEX);
    m_currScope = AstGen::GLOBAL_SCOPE_INDEX;

    DISCARD_VALUE(CreateSymbolMeta());
}

Index AstGen::CreateOrGetScopeCustom(Scope::Type type, Index parent, Index name, Index meta) {
    const auto newScope = static_cast<Index>(m_scopes.size());
    Scope& scope        = m_scopes.at(parent);

    // scope without name
    if (isNull(name)) {
        scope.Scopes.emplace_back(newScope);
        m_scopes.emplace_back(type, name, meta, parent);
        return newScope;
    }

    const Index scopeIndex = scope.Contains(name);
    if (isNull(scopeIndex)) {
        scope.Scopes.emplace_back(newScope);
        scope.NamedScopes.emplace(name, newScope);
        m_scopes.emplace_back(type, name, meta, parent);
        return newScope;
    } else {
        return scopeIndex;
    }
}

Index AstGen::GetSymbolInScope(Index name, Index scope) {
    Index curr   = scope;
    Index symbol = NULL_INDEX;

    do {
        const Scope& scopeRef = m_scopes.at(curr);

        symbol = scopeRef.Contains(name);
        curr   = scopeRef.Parent;
        if (scopeRef.Tag != Scope::BLOCK) {
            break;
        }

    } while (isNull(symbol));

    m_prevSymbol = symbol;

    return symbol;
}

void AstGen::CreateSymbol(RefInst identifier, Index decl, Index scope, Index flags) {
    if (identifier.IsConstant()) {
        KOOLANG_ERR_MSG("CANNOT USE KEYWORD AS NAME");
        return;
    }

    const Index symbol = m_scopes.at(scope).Contains(identifier.Offset);
    // symbol doesn't exist in the current block
    if (isNull(symbol)) {
        m_prevSymbol = CreateOrGetScopeCustom(Scope::SYMBOL, scope, identifier.Offset, CreateSymbolMeta(decl, flags));
        return;
    }

    const Scope& symbolScope = m_scopes.at(symbol);
    // Scope isn't symbol
    if (symbolScope.Tag != Scope::SYMBOL) {
        KOOLANG_ERR_MSG("REDECLARATION OF THE SYMBOL");
        return;
    }

    SymbolMeta& meta = m_symbolMeta.at(symbolScope.Meta);

    // redeclaration
    if (!isNull(meta.Inst)) {
        if ((meta.Flags & SymbolMeta::DISCARDED_FLAG) == 0) {
            KOOLANG_ERR_MSG("REDECLARATION OF THE SYMBOL");
            return;
        }
    }

    meta.Inst    = decl;
    meta.Flags   = flags;
    m_prevSymbol = symbol;
}

void AstGen::EnterScope(Scope::Type type, Index name, Index meta) {
    if (!isNull(name) && !isNull(m_scopes.at(m_currScope).Contains(name))) {
        KOOLANG_ERR_MSG("SCOPE ALREADY EXISTS");
        m_currScope = CreateOrGetScope(type, NULL_INDEX, meta);
    } else {
        m_currScope = CreateOrGetScope(type, name, meta);
    }
}

void AstGen::ExitScope() { m_currScope = m_scopes.at(m_currScope).Parent; }

void AstGen::AddLabel(Index str, Index inst) {
    const Index found = FindLabel(str);
    m_labels.push_back(Label { inst, str });

    if (!isNull(found)) {
        KOOLANG_ERR_MSG("DUPLICITE LABEL");
    }
}

void AstGen::PopLabel() { m_labels.pop_back(); }

Index AstGen::FindLabel(Index str) {
    Index found = NULL_INDEX;
    for (const auto& label : m_labels | std::views::reverse) {
        if (label.Str == str) {
            found = label.Inst;
            break;
        }
    }
    return found;
}

void AstGen::Generate() {
    for (const auto importNodeIndex : m_tree.Imports) {
        const auto& importNode = GetNode(importNodeIndex);
        std::string base;

        // IMPORT_PATH
        Index index = importNodeIndex + 1;

        // multiple import paths
        if (importNode.Rhs != index + 1) {
            base = pathNodeToFilePath(index + 1, m_tree);
            index += 2;
        }

        // import paths
        for (; index < importNode.Rhs; index += 2) {
            const ast::Node& importPath = GetNode(index);
            std::string path            = pathNodeToFilePath(importPath.Lhs, m_tree);

            if (!base.empty()) {
                path = base + '/' + path;
            }

            const auto strID = m_strings.GetOrCreateStr(path);
            m_kir.Imports.push_back(strID);

            if (isNull(importPath.Rhs)) {
                continue;
            }

            // import a::b = c;
            // a::b::Some == c::Some
            KOOLANG_INFO_MSG("TODO: Import alias");
            KOOLANG_TODO();
        }
    }

    const auto topBlock = EnterBlock();

    for (const auto topStmt : m_tree.Top) {
        using ast::Tag;

        const Tag tag = m_tree.NodeTags.at(topStmt);
        switch (tag) {
        case ast::Tag::CONSTANT:
            GenGlobConst(topStmt);
            break;
        case ast::Tag::FN:
            GenFn(topStmt);
            break;
        case ast::Tag::VARIANT:
            GenVariant(topStmt);
            break;
        case ast::Tag::STRUCT:
            GenStruct(topStmt);
            break;
        case ast::Tag::ENUM:
            GenEnum(topStmt);
            break;
        case ast::Tag::IMPL:
            GenImpl(topStmt);
            break;
        case ast::Tag::TRAIT:
            GenTrait(topStmt);
            break;
        default:
            KOOLANG_UNREACHABLE();
        }
    }

    CreateBlock(InstType::BLOCK, topBlock, NULL_INDEX);
}

// TOP STATEMENTS /////////////////////////////////////////////////////////////

/*
Example:

const A : u8 = 1;
---
%4 = decl("A", %1 = comptime {
    %2 = as(Ref.U8_TYPE, Ref.ONE)
    %3 = break_inline(%1, %2)
}
*/
void AstGen::GenGlobConst(Index node) {
    const RefInst strID = GetOrCreateStr(GetNodeToken(node) + 1);
    const auto& nodeRef = GetNode(node);

    const Index nodeMeta = nodeRef.Lhs;
    const Index typeNode = GetNodeMeta(nodeMeta);
    const Index vis      = GetNodeMeta(nodeMeta + 1);
    const Index docStr   = GetNodeMeta(nodeMeta + 2);

    //-- BLOCK BEGIN --//
    const InstCache block = EnterBlock();

    const RefInst exprInst  = GenExpr(nodeRef.Rhs);
    const RefInst typeInst  = GenType(typeNode);
    const RefInst valueInst = GenAs(exprInst, typeInst);

    CreateBlock(InstType::BLOCK_COMPTIME_INLINE, block, nodeRef.Rhs, valueInst);
    //-- BLOCK END --//

    const Index extra = CreateExtraFrom(extra::Decl { vis, docStr, strID.Offset });
    const Index decl  = CreateInst(InstType::DECL, InstData::CreateBin(extra, block.Inst));

    CreateSymbol(strID, decl, m_currScope, SymbolMeta::CONST_FLAG);
}

/*
Example:

pub add(a : i32, b : i32) : i32 {
    return a + b;
}
---
%11 = decl_fn("pub add",
    %1 = {
        %2 = break_inline(%1, Ref.I32_TYPE)
    },
    %3 = {
        %4 = param("a", Ref.I32_TYPE)
        %5 = param("b", Ref.I32_TYPE)
    },
    %6 = block({
        %7 = load(%4)
        %8 = load(%5)
        %9 = add(%7, %8)
        %10 = return(%9)
    })
*/
void AstGen::GenFn(Index node) {
    // can be FN_DEF or FN
    Index fnDefNodeIndex = node;
    Index fnBlockIndex   = NULL_INDEX;

    if (GetNodeTag(node) == ast::Tag::FN) {
        const auto& fnNode = GetNode(node);
        fnDefNodeIndex     = fnNode.Lhs;
        fnBlockIndex       = fnNode.Rhs;
    }

    const auto& fnDefNode = GetNode(fnDefNodeIndex);

    const Index returnType = GetNodeMeta(fnDefNode.Lhs);
    const Index modifiers  = GetNodeMeta(fnDefNode.Lhs + 1);
    const Index vis        = GetNodeMeta(fnDefNode.Lhs + 2);
    const RefInst docStr   = GetOrCreateStr(GetNodeMeta(fnDefNode.Lhs + 3));

    const RefInst fnID = GetOrCreateStr(GetNodeToken(fnDefNodeIndex) + 1);

    //-- RETURN TYPE BLOCK BEGIN --//
    InstCache returnTypeBlock = EnterBlock();
    RefInst returnTypeInst(RefInst::VOID_TYPE);

    if (!isNull(returnType)) {
        returnTypeInst = GenType(returnType);
    }

    CreateBlock(InstType::BLOCK_INLINE, returnTypeBlock, returnType, returnTypeInst);
    //-- RETURN TYPE BLOCK END --//

    EnterScope(Scope::SYMBOL, fnID.Offset);

    //-- PARAMS BLOCK BEGIN --//
    const InstCache paramBlock = EnterBlock();
    const auto& paramsNode     = GetNode(fnDefNode.Rhs);

    const Index paramsCount = paramsNode.Rhs;

    for (Index i = 0; i < paramsCount; i++) {
        const Index paramIndex = GetNodeMeta(paramsNode.Lhs + i);
        const auto& paramNode  = GetNode(paramIndex);
        const RefInst paramID  = GetOrCreateStr(GetNodeToken(paramIndex));

        const RefInst paramType = GenType(paramNode.Rhs);
        Index paramFlags        = NULL_INDEX;

        if (isNull(paramNode.Lhs)) {
            paramFlags = SymbolMeta::CONST_FLAG;
        }

        const Index paramInst = CreateInst(
            InstType::PARAM,
            InstData::CreateNodePl(paramIndex, CreateExtraFrom(extra::Param { paramID.Offset, paramType }))
        );

        CreateSymbol(paramID, paramInst, m_currScope, paramFlags);
    }

    CreateBlock(InstType::BLOCK_INLINE, paramBlock, fnDefNode.Rhs);
    //-- PARAMS BLOCK END --//

    //-- BODY BLOCK BEGIN --//
    RefInst blockInst(NULL_INDEX);

    if (!isNull(fnBlockIndex)) {
        const InstCache bodyBlock = EnterBlock();
        GenRawBlock(fnBlockIndex);
        CreateBlock(InstType::BLOCK, bodyBlock, fnBlockIndex);

        blockInst = bodyBlock.Inst;
    }
    //-- BODY BLOCK END --//
    const Index extra = CreateExtraFrom(extra::DeclFn {
        extra::Decl { vis, docStr.Offset, fnID.Offset },
        returnTypeBlock.Inst,
        modifiers,
        paramBlock.Inst,
    });

    const Index decl = CreateInst(InstType::DECL_FN, InstData::CreateBin(extra, blockInst));

    m_scopes[m_currScope].Meta = CreateSymbolMeta(decl, SymbolMeta::CONST_FLAG);
    ExitScope();
}

void AstGen::GenEnum(Index node) {
    const auto& enumNode = GetNode(node);

    const Index fieldsCount = enumNode.Rhs;

    const Index typeNode    = GetNodeMeta(enumNode.Lhs);
    const Index vis         = GetNodeMeta(enumNode.Lhs + 1);
    const RefInst docTok    = GetOrCreateStr(GetNodeMeta(enumNode.Lhs + 2));
    const Index fieldsStart = enumNode.Lhs + 3;

    const RefInst enumID = GetOrCreateStr(GetNodeToken(node) + 1);

    //-- ENUM BLOCK BEGIN --//
    const InstCache enumBlock = EnterBlock();
    EnterScope(Scope::SYMBOL, enumID.Offset);

    const RefInst type = (isNull(typeNode)) ? RefInst::U8_TYPE : GenType(typeNode);

    RefInst prevValue = NULL_INDEX;
    for (Index i = 0; i < fieldsCount; i++) {
        const Index fieldIndex = GetNodeMeta(fieldsStart + i);
        const auto& fieldNode  = GetNode(fieldIndex);
        const RefInst fieldID  = GetOrCreateStr(fieldNode.Lhs);

        if (!isNull(fieldNode.Rhs)) {

            prevValue = GenExpr(fieldNode.Rhs);

        } else if (prevValue.Offset != NULL_INDEX) {

            prevValue = CreateInst(
                InstType::ADD,
                InstData::CreateNodePl(
                    fieldIndex, CreateExtraFrom(extra::Bin { prevValue.Offset, RefInst(RefInst::ONE).Offset })
                )
            );

        } else {
            prevValue = RefInst::ZERO;
        }

        const Index field = CreateInst(
            InstType::ENUM_FIELD,
            InstData::CreateNodePl(fieldIndex, CreateExtraFrom(extra::DeclEnumField { prevValue, fieldID.Offset }))
        );

        CreateSymbol(fieldID, field, m_currScope, SymbolMeta::CONST_FLAG);
    }

    CreateBlock(InstType::BLOCK_COMPTIME_INLINE, enumBlock, node);
    //-- ENUM BLOCK END --//

    const Index extra    = CreateExtraFrom(extra::DeclEnum { extra::Decl { vis, docTok.Offset, enumID.Offset }, type });
    const Index declEnum = CreateInst(InstType::DECL_ENUM, InstData::CreateBin(extra, enumBlock.Inst));

    m_scopes[m_currScope].Meta = CreateSymbolMeta(declEnum, SymbolMeta::CONST_FLAG);
    ExitScope();
}

void AstGen::GenVariant(Index node) { DISCARD_VALUE(node); }
void AstGen::GenImpl(Index node) { DISCARD_VALUE(node); }
void AstGen::GenTrait(Index node) { DISCARD_VALUE(node); }

void AstGen::GenStruct(Index node) {
    const auto& structNode  = GetNode(node);
    const Index nodeMeta    = structNode.Lhs;
    const Index fieldsCount = structNode.Rhs;

    const Index vis         = GetNodeMeta(nodeMeta);
    const RefInst docTok    = GetOrCreateStr(GetNodeMeta(nodeMeta + 1));
    const Index fieldsStart = nodeMeta + 2;

    const RefInst structID = GetOrCreateStr(GetNodeToken(node) + 1);

    //-- STRUCT BLOCK BEGIN --//
    const InstCache structBlock = EnterBlock();
    EnterScope(Scope::SYMBOL, structID.Offset);

    for (Index i = 0; i < fieldsCount; i++) {
        const Index fieldIndex = GetNodeMeta(fieldsStart + i);
        const auto& fieldNode  = GetNode(fieldIndex);
        const ast::Tag tag     = GetNodeTag(fieldIndex);

        const RefInst fieldID = GetOrCreateStr(GetNodeToken(fieldIndex));

        const Index fieldTypeIndex = GetNodeMeta(fieldNode.Lhs);
        const Index fieldVis       = GetNodeMeta(fieldNode.Lhs + 1);
        const RefInst fieldDocTok  = GetOrCreateStr(GetNodeMeta(fieldNode.Lhs + 2));

        const RefInst fieldType = GenType(fieldTypeIndex);
        const RefInst fieldVal  = (isNull(fieldNode.Rhs)) ? NULL_INDEX : GenExpr(fieldNode.Rhs);

        const Index fieldInst = CreateInst(
            InstType::STRUCT_FIELD,
            InstData::CreateNodePl(
                fieldIndex,
                CreateExtraFrom(extra::DeclStructField { extra::Decl { fieldVis, fieldDocTok.Offset, fieldID.Offset },
                                                         fieldType, fieldVal })
            )
        );

        CreateSymbol(
            fieldID, fieldInst, m_currScope, (tag == ast::Tag::STRUCT_CONST) ? SymbolMeta::CONST_FLAG : NULL_INDEX
        );
    }

    CreateBlock(InstType::BLOCK_COMPTIME_INLINE, structBlock, node);
    //-- STRUCT BLOCK END --//

    const Index extra      = CreateExtraFrom(extra::DeclStruct { extra::Decl { vis, docTok.Offset, structID.Offset } });
    const Index declStruct = CreateInst(InstType::DECL_STRUCT, InstData::CreateBin(extra, structBlock.Inst));

    m_scopes[m_currScope].Meta = CreateSymbolMeta(declStruct, SymbolMeta::CONST_FLAG);
    ExitScope();
}

// TYPES //////////////////////////////////////////////////////////////////////

RefInst AstGen::GenType(Index node) {
    const auto& typeNode = GetNode(node);
    const auto typeTag   = GetNodeTag(typeNode.Rhs);

    RefInst inst(NULL_INDEX);
    switch (typeTag) {
    // [type;size]
    case ast::Tag::TYPE_ARR: {
        const auto& arrNode   = GetNode(typeNode.Rhs);
        const RefInst arrType = GenType(arrNode.Lhs);
        const RefInst arrSize = GenExpr(arrNode.Rhs);

        inst = CreateInst(
            InstType::ARRAY_TYPE,
            InstData::CreateNodePl(typeNode.Rhs, CreateExtraFrom(extra::ArrayType { arrSize, arrType }))
        );
        break;
    }
    // (type, type, ...)
    case ast::Tag::TYPE_TUPLE: {
        const auto& tupleNode  = GetNode(typeNode.Rhs);
        const Index extraStart = ReserveExtra(tupleNode.Rhs + 1);
        SetExtra(extraStart, tupleNode.Rhs);

        for (Index i = 0; i < tupleNode.Rhs; i++) {
            SetExtra(extraStart + i + 1, GenType(GetNodeMeta(tupleNode.Lhs + i)).Offset);
        }

        inst = CreateInst(InstType::TUPLE_TYPE, InstData::CreateNodePl(typeNode.Rhs, extraStart));
        break;
    }
    // dyn<trait + trait + ...>
    case ast::Tag::TYPE_DYNAMIC: {
        const auto& dynNode    = GetNode(typeNode.Rhs);
        const Index extraStart = ReserveExtra(dynNode.Rhs + 1);
        SetExtra(extraStart, dynNode.Rhs);

        for (Index i = 0; i < dynNode.Rhs; i++) {
            SetExtra(extraStart + i + 1, GenPath(GetNodeMeta(dynNode.Lhs + i)).Offset);
        }

        inst = CreateInst(InstType::DYN_TYPE, InstData::CreateNodePl(typeNode.Rhs, extraStart));
        break;
    }
    // fn(type, type, ...) -> type
    case ast::Tag::TYPE_FN:
        KOOLANG_TODO();
        break;
    // |[type]|
    case ast::Tag::TYPE_SLICE: {
        const RefInst baseType = GenType(typeNode.Rhs);
        inst                   = CreateInst(InstType::SLICE_TYPE, InstData::CreateNodePl(typeNode.Rhs, baseType));
        break;
    }
    // type
    case ast::Tag::PATH:
        inst = GenPath(typeNode.Rhs);
        break;
    default:
        KOOLANG_UNREACHABLE();
    }

    const Index typeMeta  = typeNode.Lhs;
    const Index ptrsCount = typeMeta & ast::TYPE_PTR_MASK;
    const Index modifiers = typeMeta & ~ast::TYPE_PTR_MASK;

    if ((modifiers & ast::TYPE_FLAG_REFERENCE) != 0) {
        inst = CreateInst(InstType::REF_TYPE, InstData::CreateNodePl(node, inst));
    }

    if (ptrsCount > 0) {
        inst = CreateInst(
            InstType::PTR_TYPE, InstData::CreateNodePl(node, CreateExtraFrom(extra::PtrType { ptrsCount, inst }))
        );
    }

    return inst;
}

RefInst AstGen::GenAs(RefInst expression, RefInst type) {
    return CreateInst(InstType::AS, InstData::CreateBin(type, expression));
}
// BLOCK STATEMENTS ///////////////////////////////////////////////////////////

void AstGen::GenRawBlock(Index node) {
    const auto& blockNode = GetNode(node);
    const Index size      = blockNode.Rhs;
    const Index meta      = blockNode.Lhs;

    for (Index i = 0; i < size; i++) {
        const Index nodeIndex = GetNodeMeta(meta + i);
        const ast::Tag tag    = GetNodeTag(nodeIndex);

        switch (tag) {
        case ast::Tag::VARIABLE:
            GenVar(nodeIndex);
            break;
        case ast::Tag::DISCARD:
            GenDiscard(nodeIndex);
            break;
        case ast::Tag::IF_STMT:
            GenIf(nodeIndex);
            break;
        case ast::Tag::CONSTANT:
            GenConst(nodeIndex);
            break;
        case ast::Tag::FLOW_OP:
            GenFlow(nodeIndex);
            break;
        case ast::Tag::FOR_STMT:
            GenFor(nodeIndex);
            break;
        case ast::Tag::WHILE_STMT:
            GenWhile(nodeIndex);
            break;
        case ast::Tag::STATIC:
            GenStatic(nodeIndex);
            break;
        default:
            DISCARD_VALUE(GenExpr(nodeIndex));
            break;
        }
    }
}

void AstGen::GenVar(Index node) {
    const auto& varNode = GetNode(node);

    const Index patternNodeIndex = GetNodeMeta(varNode.Lhs);
    const Index valueNodeIndex   = varNode.Rhs;

    const RefInst value = GenExpr(valueNodeIndex);

    GenPattern(patternNodeIndex, value);
}

void AstGen::GenPattern(Index node, RefInst value) {
    const auto& patternNode = GetNode(node);

    switch (GetNodeTag(node)) {
    // var x = 5;
    case ast::Tag::PATTERN_SINGLE: {
        const InstCache inst = PrepareInstWithCache();
        const RefInst strID  = GetOrCreateStr(GetNodeToken(node));

        const bool hasType   = !isNull(patternNode.Lhs);
        const bool isMutable = !isNull(patternNode.Rhs);

        constexpr auto lookupTable = std::to_array({
            InstType::ALLOC_INFERRED,
            InstType::ALLOC_MUT_INFERRED,
            InstType::ALLOC,
            InstType::ALLOC_MUT,
        });

        const std::uint8_t lookupTableIndex
            = static_cast<std::uint8_t>(isMutable) + static_cast<std::uint8_t>(static_cast<std::uint8_t>(hasType) << 1);
        const InstType instType = lookupTable.at(lookupTableIndex);

        const Index symbolFlags = (isMutable) ? NULL_INDEX : SymbolMeta::CONST_FLAG;
        CreateSymbol(strID, inst.Inst, m_currScope, symbolFlags);

        if (hasType) {

            const RefInst type = GenType(patternNode.Lhs);
            SetInst(inst.Inst, instType, InstData::CreateNodePl(node, type));
            DISCARD_VALUE(CreateInst(InstType::STORE, InstData::CreateBin(inst.Inst, value)));
        } else {
            SetInst(inst.Inst, instType, InstData::CreateRef(node));
            DISCARD_VALUE(CreateInst(InstType::STORE_INFERRED, InstData::CreateBin(inst.Inst, value)));
        }
        break;
    }

    // var (x,y) = (1,2);
    case ast::Tag::PATTERN_MULTIPLE: {
        const Index meta = patternNode.Lhs;
        const Index size = patternNode.Rhs;

        for (Index i = 0; i < size; i++) {
            // field access
            const RefInst tmpInst = CreateInst(InstType::FIELD_SHORT, InstData::CreateBin(value, i));

            GenPattern(GetNodeMeta(meta + i), tmpInst);
        }
        break;
    }
    // var Vector2D{ x -> varX, y -> varY } = ...;
    case ast::Tag::PATTERN_STRUCT: {
        /*
        1. Get struct path
        2. Cast the value to the struct
        3. Iterate all fields
        */

        // 1.
        const RefInst path     = GenPath(patternNode.Lhs);
        const auto& fieldsNode = GetNode(patternNode.Rhs);

        if (path.IsConstant()) {
            // var i32{ ... } = ...
            KOOLANG_ERR_MSG("CANNOT USE KEYWORD AS STRUCT PATH");
            return;
        }

        // 2.
        const RefInst asInst = GenAs(value, path);

        // 3.
        for (Index i = 0; i < fieldsNode.Rhs; i++) {
            const Index fieldNodeIndex = GetNodeMeta(fieldsNode.Lhs + i);
            const auto& fieldNode      = GetNode(fieldNodeIndex);

            const bool isMutable = !isNull(fieldNode.Rhs);

            const RefInst fieldID = GetOrCreateStr(fieldNode.Lhs);
            const RefInst varID   = GetOrCreateStr(GetNodeToken(fieldNodeIndex));

            if (fieldID.IsConstant()) {
                KOOLANG_ERR_MSG("CANNOT USE KEYWORD AS THE FIELD NAME");
                return;
            }

            const Index fieldNameInst = CreateInst(InstType::IDENT, fieldID);
            const Index accessInst    = CreateInst(InstType::FIELD_SHORT, InstData::CreateBin(asInst, fieldNameInst));

            const InstType instType = (isMutable) ? InstType::ALLOC_MUT_INFERRED : InstType::ALLOC_INFERRED;
            const Index symbolFlags = (isMutable) ? NULL_INDEX : SymbolMeta::CONST_FLAG;

            const Index allocInst = CreateInst(instType, InstData::CreateRef(fieldNodeIndex));
            DISCARD_VALUE(CreateInst(InstType::STORE_INFERRED, InstData::CreateBin(allocInst, accessInst)));

            CreateSymbol(varID, allocInst, m_currScope, symbolFlags);
        }
        break;
    }

    // var (x, _) = (5, false);
    case ast::Tag::PATTERN_DISCARD:
        break;
    default:
        KOOLANG_UNREACHABLE();
    }
}

void AstGen::GenDiscard(Index node) {
    const auto& discardNode = GetNode(node);

    m_prevSymbol = NULL_INDEX;

    const ast::Tag exprTag = GetNodeTag(discardNode.Rhs);
    const RefInst expr     = GenExpr(discardNode.Rhs);

    if (expr.IsConstant()) {
        KOOLANG_ERR_MSG("CANNOT DISCARD CONSTANT/KEYWORD");
        return;
    } else if (exprTag != ast::Tag::PATH) {
        return;
    }

    const auto& pathNode = GetNode(discardNode.Rhs);

    if (pathNode.Lhs != pathNode.Rhs || isNull(m_prevSymbol)) {
        KOOLANG_ERR_MSG("CANNOT DISCARD EXTERN VARIABLE");
        return;
    }

    const Index symbol      = m_prevSymbol;
    const auto& symbolScope = m_scopes.at(symbol);
    auto& symbolMeta        = m_symbolMeta.at(symbolScope.Meta);

    symbolMeta.Flags |= SymbolMeta::DISCARDED_FLAG;

    DISCARD_VALUE(CreateInst(InstType::DISCARD_DESTRUCTOR, InstData::CreateRef(expr)));
}

/*
Example:
var x : i32 = 0;
if (a)
{
    x = 1;
}
else if (b)
{
    x = 2;
}
else
{
    x = 3;
}
---
%1 = alloc(Ref.I32_TYPE)
%2 = store(%1, Ref.ZERO)
-- 1. if
%3 = load(a)
%4 = as(Ref.BOOL_TYPE, %3)
%5 = condbr(%4, body_len=3, end=%19)
%6 = block({
  %7 = load(%1)
  %8 = store_node(%7, Ref.ONE)
})
-- 2. else if
%9 = load(b)
%10 = as(Ref.BOOL_TYPE, %9)
%11 = condbr(%10, body_len=4, end=%19)
%12 = block({
  %13 = load(%1)
  %14 = int(2)
  %15 = store_node(%13, %14)
})
-- 3. else
%16 = block({
  %17 = load(%1)
  %18 = int(3)
  %19 = store_node(%17, %18)
})
*/
void AstGen::GenIf(Index node) {
    const auto& ifNode = GetNode(node);

    const Index blockNodeIndex = ifNode.Rhs;

    const Index condNodeIndex = GetNodeMeta(ifNode.Lhs);
    const Index nextIfNode    = GetNodeMeta(ifNode.Lhs + 1);

    const RefInst expr     = GenExpr(condNodeIndex);
    const RefInst condExpr = GenAs(expr, RefInst::BOOL_TYPE);
    const InstCache condbr = PrepareInstWithCache();

    //-- BLOCK BEGIN --//
    const InstCache block = EnterBlock();
    EnterScope(Scope::BLOCK);

    GenRawBlock(blockNodeIndex);

    ExitScope();
    CreateBlock(InstType::BLOCK, block, blockNodeIndex);
    //-- BLOCK END --//

    // blocks aren't cached
    AddToCache(block.Inst);

    // Reserve space for the count instructions and else instruction
    const Index extraStart = ReserveExtra(2);
    const auto bodyLen     = static_cast<Index>(m_kir.Inst.size() - condbr.Inst - 1);

    SetInst(condbr.Inst, InstType::CONDBR, InstData::CreateBin(condExpr, extraStart));

    const ast::Tag tag = GetNodeTag(nextIfNode);
    // multiple branches/ifs
    if (tag == ast::Tag::IF_STMT) {
        GenIf(nextIfNode);
    }
    // else block
    else if (tag == ast::Tag::BLOCK) {
        //-- ELSE BLOCK BEGIN --//
        const InstCache elseBlock = EnterBlock();
        EnterScope(Scope::BLOCK);

        GenRawBlock(nextIfNode);

        ExitScope();
        CreateBlock(InstType::BLOCK, elseBlock, nextIfNode);
        //-- ELSE BLOCK END --//

        AddToCache(elseBlock.Inst);
    }

    const auto lastInst = static_cast<Index>(m_kir.Inst.size() - 1);

    SetExtra(extraStart, bodyLen);
    SetExtra(extraStart + 1, lastInst);
}

void AstGen::GenConst(Index node) {
    const auto& constNode = GetNode(node);
    const RefInst strID   = GetOrCreateStr(GetNodeToken(node) + 1);

    const Index typeNode = GetNodeMeta(constNode.Lhs);

    //-- COMPTIME BLOCK BEGIN --//
    const InstCache block = EnterBlock();

    const RefInst expr = GenExpr(constNode.Rhs);

    RefInst blockReturn = expr;
    if (!isNull(typeNode)) {
        const RefInst typeInst = GenType(typeNode);
        blockReturn            = GenAs(expr, typeInst);
    }

    CreateBlock(InstType::BLOCK_COMPTIME_INLINE, block, node, blockReturn);
    //-- COMPTIME BLOCK END --//

    CreateSymbol(strID, block.Inst, m_currScope, SymbolMeta::CONST_FLAG);
}

void AstGen::GenFlow(Index node) {
    const auto& flowNode = GetNode(node);

    // label or expression
    const Index valueIndex = flowNode.Rhs;

    if (flowNode.Lhs == ast::FLOW_RETURN) {
        DISCARD_VALUE(CreateInst(InstType::RETURN, InstData::CreateNodePl(node, GenExpr(valueIndex))));
        return;
    }

    const RefInst labelID = GetOrCreateStr(valueIndex);
    const Index loopInst  = FindLabel(labelID.Offset);
    if (isNull(loopInst)) {
        KOOLANG_ERR_MSG("LABEL NOT FOUND");
    }

    const InstType instType = (flowNode.Lhs == ast::FLOW_CONTINUE) ? InstType::CONTINUE : InstType::BREAK;

    DISCARD_VALUE(CreateInst(instType, InstData::CreateNodePl(node, loopInst)));
}

/*
Example:
for x in elements {
    // ...
}
---

%1 = indexable_len(elements) -> n
%2 = alloc_mut(Ref.USIZE_TYPE) -> var i : usize = 0
%3 = store(%2, Ref.ZERO)
%4 = loop({
    %5 = load(%2)
    %6 = cmp_ls(%5, %1)
    %7 = condbr(%6, body_len=7, end=%14) -> i < n
    %8 = block({
        %9 = arr_el(elements, %2) -> elements[i]
        %10 = alloc_mut_inferred() -> var x = elements[i]
        %11 = store_inferred(%10, %9)
        %12 = add(%2, Ref.ONE) -> i += 1
        %13 = store(%2, %12)
        %14 = repeat(%4)
    })
})
*/
void AstGen::GenFor(Index node) {
    const auto& forNode        = GetNode(node);
    const Index meta           = forNode.Lhs;
    const Index blockNodeIndex = forNode.Rhs;

    const Index patternNodeIndex  = GetNodeMeta(meta);
    const Index iterableNodeIndex = GetNodeMeta(meta + 1);
    const Index labelTok          = GetNodeMeta(meta + 2);

    const RefInst elements = GenExpr(iterableNodeIndex);

    // %1
    const Index indexableLen = CreateInst(InstType::INDEXABLE_LEN, InstData::CreateRef(elements));
    // %2
    const Index varIndex = CreateInst(InstType::ALLOC_MUT, InstData::CreateNodePl(node, RefInst::USIZE_TYPE));
    // %3
    DISCARD_VALUE(CreateInst(InstType::STORE, InstData::CreateBin(varIndex, RefInst::ZERO)));

    //-- LOOP BEGIN --//
    // %4
    const InstCache loopBlock = EnterBlock();
    if (!isNull(labelTok)) {
        AddLabel(GetOrCreateStr(labelTok).Offset, loopBlock.Inst);
    }

    // %5
    const Index loadInst = CreateInst(InstType::LOAD, InstData::CreateRef(varIndex));
    // %6
    const RefInst cmpInst = CreateInst(
        InstType::CMP_LS, InstData::CreateNodePl(node, CreateExtraFrom(extra::Bin { loadInst, indexableLen }))
    );

    // %7
    const InstCache condbr = PrepareInstWithCache();

    //-- BLOCK BEGIN --//
    // %8
    const InstCache block = EnterBlock();
    EnterScope(Scope::BLOCK);

    const Index accessElement = CreateInst(
        InstType::ARR_EL,
        InstData::CreateNodePl(iterableNodeIndex, CreateExtraFrom(extra::Bin { elements.Offset, varIndex }))
    );

    DISCARD_VALUE(GenPattern(patternNodeIndex, accessElement));

    GenRawBlock(blockNodeIndex);

    // %9
    const Index add = CreateInst(
        InstType::ADD,
        InstData::CreateNodePl(node, CreateExtraFrom(extra::Bin { varIndex, RefInst(RefInst::ONE).Offset }))
    );
    // %10
    DISCARD_VALUE(CreateInst(InstType::STORE, InstData::CreateBin(varIndex, add)));
    // %11
    const Index repeat = CreateInst(InstType::REPEAT, InstData::CreateRef(loopBlock.Inst));

    const auto bodyLen = static_cast<Index>(m_kir.Inst.size() - block.Inst);
    SetInst(
        condbr.Inst, InstType::CONDBR, InstData::CreateBin(cmpInst, CreateExtraFrom(extra::IfData { bodyLen, repeat }))
    );

    ExitScope();
    CreateBlock(InstType::BLOCK, block, blockNodeIndex);
    AddToCache(block.Inst);
    //-- BLOCK END --//

    if (!isNull(labelTok)) {
        PopLabel();
    }
    CreateBlock(InstType::LOOP, loopBlock, node);
    AddToCache(loopBlock.Inst);
    //-- LOOP END --//
}

/*
Example:

while(true) {
    // ...
}
---
%1 = loop({
    %2 = as(Ref.BOOL_TYPE, Ref.BOOL_TRUE)
    %3 = condbr(%2, body_len=2, end=%5)
    %4 = block({
        %5 = repeat(%1)
    })
})
*/
void AstGen::GenWhile(Index node) {
    const auto& whileNode      = GetNode(node);
    const Index meta           = whileNode.Lhs;
    const Index blockNodeIndex = whileNode.Rhs;

    const Index conditionNode = GetNodeMeta(meta);
    const Index labelTok      = GetNodeMeta(meta + 1);

    //-- LOOP BEGIN --//
    const InstCache loopBlock = EnterBlock();
    if (!isNull(labelTok)) {
        AddLabel(GetOrCreateStr(labelTok).Offset, loopBlock.Inst);
    }

    const RefInst condition = GenExpr(conditionNode);
    const RefInst castCond  = GenAs(condition, RefInst::BOOL_TYPE);

    const InstCache condbr = PrepareInstWithCache();

    //-- BLOCK BEGIN --//
    const InstCache block = EnterBlock();
    EnterScope(Scope::BLOCK);

    GenRawBlock(blockNodeIndex);

    const Index repeat = CreateInst(InstType::REPEAT, InstData::CreateRef(loopBlock.Inst));

    const auto bodyLen = static_cast<Index>(m_kir.Inst.size() - block.Inst);
    SetInst(
        condbr.Inst, InstType::CONDBR, InstData::CreateBin(castCond, CreateExtraFrom(extra::IfData { bodyLen, repeat }))
    );

    ExitScope();
    CreateBlock(InstType::BLOCK, block, blockNodeIndex);
    AddToCache(block.Inst);
    //-- BLOCK END --//

    if (!isNull(labelTok)) {
        PopLabel();
    }
    CreateBlock(InstType::LOOP, loopBlock, node);
    AddToCache(loopBlock.Inst);
    //-- LOOP END --//
}

void AstGen::GenStatic(Index node) {
    KOOLANG_TODO();
    DISCARD_VALUE(node);
}

// EXPRESSIONS ////////////////////////////////////////////////////////////////

RefInst AstGen::GenExpr(Index node) {
    const ast::Tag tag = GetNodeTag(node);

    switch (tag) {
    case ast::Tag::PATH:
        return GenPath(node);
    case ast::Tag::LITERAL:
        return GenLiteral(node);
    case ast::Tag::ARRAY:
        return GenArray(node);
    case ast::Tag::ARRAY_SHORT:
        return GenArrayShort(node);
    case ast::Tag::TUPLE:
        return GenTuple(node);
    case ast::Tag::CAST_EXPR:
        return GenCast(node);
    case ast::Tag::CLOSURE_EXPR:
        return GenClosure(node);
    case ast::Tag::SINGLE_OP:
        return GenSingleOp(node);
    case ast::Tag::UNWRAP_OP:
        return GenUnwrap(node);
    case ast::Tag::BIN_OP:
        return GenBinOp(node);
    case ast::Tag::SLICE_OP:
        return GenSlice(node);
    case ast::Tag::CALL_OP:
        return GenCall(node);
    case ast::Tag::STRUCT_EXPR:
        return GenStructExpr(node);
    case ast::Tag::GROUPED_EXPR:
        return GenExpr(GetNode(node).Rhs);
    default:
        KOOLANG_UNREACHABLE();
    }
}

RefInst AstGen::GenPath(Index node) {
    /*

     Start              End
     |   0   |  1 |   2   |
     |-------|----|-------|
     | IDENT | :: | IDENT |

     End - Start = The count of the tokens in between
     (End - Start) / 2 = The count of identifiers (0 to (n - 1))
    */

    const auto& pathNode   = GetNode(node);
    const Index partsCount = (pathNode.Rhs - pathNode.Lhs) / 2;
    const RefInst lastID   = GetOrCreateStr(pathNode.Rhs);

    Index currScope        = m_currScope;
    const Index extraStart = GetExtraSize();

    if (partsCount > 0) {
        AddToExtra(partsCount);
    }

    // process all identifiers except the last one
    for (Index i = 0; i < partsCount; i++) {
        const RefInst id  = GetOrCreateStr(pathNode.Lhs + i * 2);
        const Index scope = m_scopes.at(currScope).Contains(id.Offset);

        if (isNull(scope)) {
            currScope = CreateOrGetScopeCustom(Scope::SYMBOL, currScope, id.Offset);
        } else {
            currScope = scope;
        }

        AddToExtra(id.Offset);
    }

    RefInst inst = NULL_INDEX;

    // A::B::C
    if (partsCount > 0) {
        const Index namespaceInst = CreateInst(InstType::NAMESPACE, InstData::CreateNodePl(node, extraStart));

        inst = CreateInst(
            InstType::DECL_ITEM,
            InstData::CreateTokPl(pathNode.Rhs, CreateExtraFrom(extra::DeclItem { lastID.Offset, namespaceInst }))
        );

        if (lastID.IsConstant()) {
            // A::B::i32 -> error
            KOOLANG_ERR_MSG("CANNOT USE KEYWORD");
        }

        return inst;

    }
    // keywords
    else if (lastID.IsConstant()) {
        return lastID;
    }

    // local variable
    const Index symbol = GetSymbolInScope(lastID.Offset, m_currScope);
    m_prevSymbol       = symbol;
    // not defined
    if (isNull(symbol)) {
        return CreateInst(InstType::DECL_REF, InstData::CreateTokPl(pathNode.Rhs, lastID));
    }

    const Index metaIndex  = m_scopes.at(symbol).Meta;
    const auto& symbolMeta = m_symbolMeta.at(metaIndex);

    if (isNull(symbolMeta.Inst)) {
        return CreateInst(InstType::DECL_REF, InstData::CreateTokPl(pathNode.Rhs, lastID));
    } else {
        return CreateInst(InstType::LOAD, InstData::CreateRef(symbolMeta.Inst));
    }
}

RefInst AstGen::GenLiteral(Index node) {
    const auto& literalNode = GetNode(node);

    std::string_view content = m_tree.Tokens.GetTokContent(literalNode.Rhs);

    switch (literalNode.Lhs) {
    case ast::LITERAL_STRING:
        return CreateInst(InstType::STR, InstData::CreateStrTok(GetOrCreateStr(content).Offset, literalNode.Rhs));
    case ast::LITERAL_CHAR:
        return CreateInst(InstType::CHAR, InstData::CreateStrTok(GetOrCreateStr(content).Offset, literalNode.Rhs));

    case ast::LITERAL_FLOAT: {
        bool err;
        auto value = convertToF64(content, err);
        if (err) {
            KOOLANG_TODO_EXIT("FLOAT CANNOT FIT F64");
        }

        return CreateInst(InstType::FLOAT, InstData::CreateFloat(value));
    }

    case ast::LITERAL_NUMBER: {
        bool err;
        std::uint64_t value = convertToU64(content, err);
        if (err) {
            KOOLANG_TODO_EXIT("TOO BIT INT");
        } else if (value == 0) {
            return RefInst::ZERO;
        } else if (value == 1) {
            return RefInst::ONE;
        } else {
            return CreateInst(InstType::INT, InstData::CreateInt(value));
        }
    }
    }

    KOOLANG_UNREACHABLE();
}

RefInst AstGen::GenArray(Index node) {
    const auto& arrNode = GetNode(node);
    const Index size    = arrNode.Rhs;
    const Index meta    = arrNode.Lhs;

    const Index arrMeta = ReserveExtra(size + 1);
    SetExtra(arrMeta, size);

    for (Index i = 0; i < size; i++) {
        SetExtra(arrMeta + i + 1, GenExpr(GetNodeMeta(meta + i)).Offset);
    }

    return CreateInst(InstType::ARR_INIT, InstData::CreateNodePl(node, arrMeta));
}

RefInst AstGen::GenArrayShort(Index node) {
    const auto& arrNode = GetNode(node);
    const RefInst size  = GenExpr(arrNode.Rhs);
    const RefInst expr  = GenExpr(arrNode.Lhs);

    return CreateInst(
        InstType::ARR_SHORT_INIT,
        InstData::CreateNodePl(node, CreateExtraFrom(extra::ArrayType { size.Offset, expr.Offset }))
    );
}

RefInst AstGen::GenTuple(Index node) {
    const auto& tupleNode = GetNode(node);
    const Index size      = tupleNode.Rhs;

    const Index extraStart = ReserveExtra(size + 1);
    SetExtra(extraStart, size);

    for (Index i = 0; i < size; i++) {
        SetExtra(extraStart + i + 1, GenExpr(GetNodeMeta(tupleNode.Lhs + i)).Offset);
    }

    return CreateInst(InstType::TUPLE, InstData::CreateNodePl(node, extraStart));
}

RefInst AstGen::GenCast(Index node) {
    const auto& castNode = GetNode(node);
    const RefInst type   = GenType(castNode.Lhs);
    const RefInst value  = GenExpr(castNode.Rhs);

    return CreateInst(InstType::CAST, InstData::CreateBin(type, value));
}

RefInst AstGen::GenClosure(Index node) {
    DISCARD_VALUE(node);
    KOOLANG_TODO();
    return NULL_INDEX;
}

RefInst AstGen::GenSingleOp(Index node) {
    const auto& opNode   = GetNode(node);
    const auto operation = static_cast<ast::SingleOp>(opNode.Lhs);
    InstType type;

    switch (operation) {
    case ast::SingleOp::BOOL_NEG:
        type = InstType::BOOL_NEG;
        break;
    case ast::SingleOp::BIT_NEG:
        type = InstType::BIT_NEG;
        break;
    case ast::SingleOp::GET_ADDR:
        type = InstType::GET_ADDR;
        break;
    case ast::SingleOp::INT_NEG:
        type = InstType::INT_NEG;
        break;
    case ast::SingleOp::DEREF:
        type = InstType::DEREF;
        break;
    }

    return CreateInst(type, InstData::CreateNodePl(node, GenExpr(opNode.Rhs)));
}

RefInst AstGen::GenUnwrap(Index node) {
    const auto& unwrapNode = GetNode(node);
    return CreateInst(InstType::UNWRAP, InstData::CreateNodePl(node, GenExpr(unwrapNode.Rhs)));
}

RefInst AstGen::GenSlice(Index node) {
    const auto& astnode = GetNode(node);
    const RefInst base  = GenExpr(astnode.Rhs);

    const Index metaIndex = astnode.Lhs;
    const Index fromExpr  = GetNodeMeta(metaIndex);
    const Index toExpr    = GetNodeMeta(metaIndex + 1);

    if (isNull(fromExpr)) {
        return CreateInst(
            InstType::SLICE_END,
            InstData::CreateNodePl(node, CreateExtraFrom(extra::Bin { base.Offset, GenExpr(toExpr).Offset }))
        );
    } else if (isNull(toExpr)) {
        return CreateInst(
            InstType::SLICE_START,
            InstData::CreateNodePl(node, CreateExtraFrom(extra::Bin { base.Offset, GenExpr(fromExpr).Offset }))
        );
    }

    const RefInst fromInst = GenExpr(fromExpr);
    const RefInst toInst   = GenExpr(toExpr);

    return CreateInst(
        InstType::SLICE_FULL,
        InstData::CreateNodePl(node, CreateExtraFrom(extra::Slice { base.Offset, fromInst.Offset, toInst.Offset }))
    );
}

RefInst AstGen::GenCall(Index node) {
    const auto& callNode  = GetNode(node);
    const Index argsCount = GetNodeMeta(callNode.Lhs);

    const RefInst base = GenExpr(callNode.Rhs);

    if (argsCount == 0) {
        return CreateInst(
            InstType::CALL, InstData::CreateNodePl(node, CreateExtraFrom(extra::Call { base.Offset, 0 }))
        );
    }

    const Index callExtra     = CreateExtraFrom(extra::Call { base.Offset, argsCount });
    const Index reservedStart = ReserveExtra(argsCount);

    for (Index i = 0; i < argsCount; i++) {
        SetExtra(reservedStart + i, GenExpr(GetNodeMeta(callNode.Lhs + i + 1)).Offset);
    }

    return CreateInst(InstType::CALL, InstData::CreateNodePl(node, callExtra));
}

RefInst AstGen::GenStructExpr(Index node) {
    const auto& structNode = GetNode(node);

    const RefInst structPath = GenPath(structNode.Lhs);

    if (isNull(structNode.Rhs)) {
        return CreateInst(InstType::STRUCT_INIT_EMPTY, InstData::CreateNodePl(node, structPath));
    }

    // STRUCT_EXPR_FIELDS
    const auto& fields      = GetNode(structNode.Rhs);
    const Index fieldsCount = fields.Rhs;
    const Index fieldsMeta  = fields.Lhs;

    // (name + value) * fields = fields * 2
    const Index meta = ReserveExtra(fieldsCount * 2 + 2);
    SetExtra(meta, structPath.Offset);
    SetExtra(meta + 1, fieldsCount);

    for (Index i = 0; i < fieldsCount; i++) {
        const Index fieldIndex = GetNodeMeta(fieldsMeta + i);
        // STRUCT_EXPR_FIELD
        const ast::Node field = GetNode(fieldIndex);

        SetExtra(meta + i * 2 + 2, GetOrCreateStr(field.Lhs).Offset);
        SetExtra(meta + i * 2 + 3, GenExpr(field.Rhs).Offset);
    }

    return CreateInst(InstType::STRUCT_INIT, InstData::CreateNodePl(node, meta));
}

RefInst AstGen::GenField(Index rhsNode, RefInst lhs) {
    const auto& pathNode = GetNode(rhsNode);

    const RefInst ident = GetOrCreateStr(pathNode.Lhs);
    const RefInst inst  = CreateInst(InstType::IDENT, InstData::CreateRef(ident));

    return CreateInst(
        InstType::FIELD, InstData::CreateNodePl(rhsNode, CreateExtraFrom(extra::FieldExpr { lhs, inst }))
    );
}

RefInst AstGen::GenBinOp(Index node) {
    using Op = ast::Operators;

    const auto& binNode  = GetNode(node);
    const auto operation = static_cast<Op>(GetNodeToken(node));

    // && operation
    if (operation == Op::AND_AND) {
        const RefInst lhs = GenExpr(binNode.Lhs);
        const Index inst  = PrepareInst();
        AddToCache(inst);

        //-- BLOCK BEGIN --//
        const InstCache block = EnterBlock();
        const RefInst rhs     = GenExpr(binNode.Rhs);
        CreateBlock(InstType::BLOCK_INLINE, block, binNode.Rhs, rhs);
        //-- BLOCK END --//

        SetInst(
            inst, InstType::LOGIC_AND,
            InstData::CreateNodePl(node, CreateExtraFrom(extra::Bin { lhs.Offset, block.Inst }))
        );
        return inst;
    }
    // || operation
    else if (operation == Op::OR_OR) {
        const RefInst lhs = GenExpr(binNode.Lhs);
        const Index inst  = PrepareInst();
        AddToCache(inst);

        //-- BLOCK BEGIN --//
        const InstCache block = EnterBlock();
        const RefInst rhs     = GenExpr(binNode.Rhs);
        CreateBlock(InstType::BLOCK_INLINE, block, binNode.Rhs, rhs);
        //-- BLOCK END --//

        SetInst(
            inst, InstType::LOGIC_OR,
            InstData::CreateNodePl(node, CreateExtraFrom(extra::Bin { lhs.Offset, block.Inst }))
        );
        return inst;
    }

    RefInst lhs = GenExpr(binNode.Lhs);

    // a.b operation
    if (operation == Op::ACCESS) {
        return GenField(binNode.Rhs, lhs);
    }
    // a->b operation
    else if (operation == Op::ACCESS_PTR) {
        lhs = CreateInst(InstType::DEREF, InstData::CreateNodePl(node, lhs));
        return GenField(binNode.Rhs, lhs);
    }

    const RefInst rhs = GenExpr(binNode.Rhs);

    InstType type;
    bool storeVal = false;
    switch (operation) {

    case Op::EQ_ADD:
        storeVal = true;
        [[fallthrough]];
    case Op::ADD:
        type = InstType::ADD;
        break;

    case Op::EQ_MUL:
        storeVal = true;
        [[fallthrough]];
    case Op::MUL:
        type = InstType::MUL;
        break;

    case Op::EQ_MOD:
        storeVal = true;
        [[fallthrough]];
    case Op::MOD:
        type = InstType::MOD;
        break;

    case Op::EQ_DIV:
        storeVal = true;
        [[fallthrough]];
    case Op::DIV:
        type = InstType::DIV;
        break;

    case Op::EQ_SUB:
        storeVal = true;
        [[fallthrough]];
    case Op::SUB:
        type = InstType::SUB;
        break;

    case Op::ACCESS_ARR:
        type = InstType::ARR_EL;
        break;

    case Op::LS:
        type = InstType::CMP_LS;
        break;

    case Op::GT:
        type = InstType::CMP_GT;
        break;

    case Op::LS_EQ:
        type = InstType::CMP_LSE;
        break;

    case Op::GT_EQ:
        type = InstType::CMP_GTE;
        break;

    case Op::NOT_EQ:
        type = InstType::CMP_NEQ;
        break;

    case Op::EQ_EQ:
        type = InstType::CMP_EQ;
        break;

    case Op::EQ_AND:
        storeVal = true;
        [[fallthrough]];
    case Op::AND:
        type = InstType::BIT_AND;
        break;

    case Op::EQ_OR:
        storeVal = true;
        [[fallthrough]];
    case Op::OR:
        type = InstType::BIT_OR;
        break;

    case Op::EQ_XOR:
        storeVal = true;
        [[fallthrough]];
    case Op::XOR:
        type = InstType::BIT_XOR;
        break;
    case Op::SHIFT_L:
        type = InstType::BIT_SHL;
        break;
    case Op::SHIFT_R:
        type = InstType::BIT_SHR;
        break;
    case Op::EQ:
        return CreateInst(
            InstType::STORE_NODE, InstData::CreateNodePl(node, CreateExtraFrom(extra::Bin { lhs.Offset, rhs.Offset }))
        );
    // Cannot happen
    case Op::AND_AND:
    case Op::OR_OR:
    case Op::INVALID:
    case Op::ACCESS:
    case Op::ACCESS_PTR:
    case Op::CALL:
    case Op::UNWRAP:
        KOOLANG_UNREACHABLE();
    }

    auto inst = CreateInst(type, InstData::CreateNodePl(node, CreateExtraFrom(extra::Bin { lhs.Offset, rhs.Offset })));
    if (storeVal) {
        return CreateInst(InstType::STORE, InstData::CreateBin(lhs, inst));
    }

    return inst;
}
}
