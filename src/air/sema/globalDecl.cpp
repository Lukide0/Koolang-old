#include "Sema.h"
#include "kir/Extra.h"

namespace air {

void Sema::KirGlobConst(Index compBlockIndex) {
    AnalyzeBlock(compBlockIndex);
    // TODO: Remove all instructions except the last
}
void Sema::KirGlobStatic(Index blockIndex) {
    AnalyzeBlock(blockIndex);

    KOOLANG_TODO();
    KOOLANG_UNREACHABLE();
}
void Sema::KirGlobFnDecl() {
    Index fnDeclIndex = m_kir.Inst.at(m_kirInst).Bin.Lhs.Offset;
    auto fnDecl       = GetKirData<kir::extra::DeclFn>(fnDeclIndex);
    DISCARD_VALUE(fnDecl);

    // TODO: process return type, ...
}
void Sema::KirGlobFnBody() { KirGlobFnDecl(); }
void Sema::KirGlobEnum() { }
void Sema::KirGlobStruct() { }
void Sema::KirGlobVariant() { }

}
