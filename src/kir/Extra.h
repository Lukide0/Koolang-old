#ifndef KOOLANG_KIR_EXTRA_H
#define KOOLANG_KIR_EXTRA_H

#include "RefInst.h"
#include "util/Index.h"
namespace kir::extra {

struct Block {
    // after this field there are trailing instructions
    Index InstCount;
};

struct Decl {
    Index Vis;
    Index DocStr;
    Index Name;
};

struct DeclFn {
    Decl DeclInfo;
    Index RetTypeInst;
    Index Modifiers; // 0 = None, 1 = const, 2 = mut
    Index Params;
};

struct DeclEnum {
    Decl DeclInfo;
    RefInst Type;
};

struct DeclVariant {
    Decl DeclInfo;
};

struct DeclStruct {
    Decl DeclInfo;
};

struct DeclStructField {
    Decl DeclInfo;
    RefInst Type;
    RefInst DefaultValue;
};

struct DeclEnumField {
    RefInst Value;
    Index Name;
};

struct DeclTrait {
    Decl DeclInfo;
};

struct DeclImpl {
    Decl DeclInfo;
    RefInst TraitPath;
    RefInst StructPath;
};

struct Bin {
    Index Lhs;
    Index Rhs;
};

struct DeclItem {
    Index Name;
    Index NamespaceInst;
};

struct Call {
    Index Base;
    // after this field there are trailing instructions
    Index Argc;
};

struct Slice {
    Index Base;
    Index From;
    Index To;
};

struct StructInit {
    Index PathInst;

    // after this field there are trailing pairs {field name, value}
    Index FieldsCount;
};

struct Param {
    Index Name;
    RefInst Type;
};

struct ArrayType {
    RefInst Size;
    RefInst TypeInst;
};

struct PtrType {
    Index Count;
    RefInst TypeInst;
};

struct ArrayShortInit {
    Index Size;
    Index ValueInst;
};

struct IfData {
    Index BodyLen;
    Index End;
};

struct FieldExpr {
    RefInst Base;
    RefInst Field;
};

}

#endif
