#ifndef KOOLANG_AIR_INSTDATA_H
#define KOOLANG_AIR_INSTDATA_H

#include "util/Index.h"
#include "util/alias.h"
namespace air {

namespace data {
    struct TyOp {
        Index Ty;
        Index Operand;
    };

    struct Symbol {
        Index Decl;
        Index Ty;
    };

    struct Bin {
        Index Lhs;
        Index Rhs;
    };
}

union InstData {
    Index Data;
    data::TyOp TyOp;
    data::Symbol Sym;
    data::Bin BinOp;

    InstData(Index poolIndex)
        : Data(poolIndex) { }

    InstData(data::TyOp tyOp)
        : TyOp(tyOp) { }

    InstData(data::Symbol symbol)
        : Sym(symbol) { }

    InstData(data::Bin bin)
        : BinOp(bin) { }

    InstData static CreatePoolIndex(Index index) { return { index }; }
    InstData static CreateData(Index index) { return { index }; }
    InstData static CreateSymbol(Index decl, Index ty) { return data::Symbol { decl, ty }; }
    InstData static CreateTyOp(Index ty, Index op) { return { data::TyOp { ty, op } }; }
    InstData static CreateBinOp(Index lhs, Index rhs) { return { data::Bin { lhs, rhs } }; }
};

ASSERT_8BYTES(InstData);

}

#endif
