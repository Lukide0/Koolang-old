#ifndef KOOLANG_KIR_INSTDATA_H
#define KOOLANG_KIR_INSTDATA_H

#include "RefInst.h"
#include "util/Index.h"
#include "util/alias.h"
#include <cstdint>

namespace kir {

/// UNION FIELD TYPES /////////////////////////////////////////////////////////////////////////////
namespace data {
    struct Bin {
        RefInst Lhs;
        RefInst Rhs;
    };

    struct UBin {
        RefInst Node;
        RefInst Op;

        enum class Op : std::uint8_t {
            LOAD_FROM_PTR,
            TEST_NULL_PTR,
            TEST_NOT_NULL_PTR,
        };
    };

    struct NodePl {
        Index NodeOffset;

        RefInst Payload;
    };

    struct LineColumn {
        Index Line;
        Index Column;
    };

    struct TokPl {
        Index TokenOffset;
        RefInst Payload;
    };

    struct StrTok {
        Index Str;
        Index Tok;
    };

    struct StrOp {
        Index Str;
        RefInst Op;
    };
}
///////////////////////////////////////////////////////////////////////////////////////////////////
union InstData {
    // Node with payload
    data::NodePl NodePl;
    data::TokPl TokPl;

    // 64bit number
    std::uint64_t Int;
    double Float;

    // Index of a string
    Index Str;

    // Token with index of a string
    data::StrTok StrTok;

    data::LineColumn DbgStmt;

    // Binary operation
    data::Bin Bin;

    // Reference
    RefInst Ref;

    // Constructors
    InstData() { }
    InstData(data::NodePl val)
        : NodePl(val) { }
    InstData(data::TokPl val)
        : TokPl(val) { }
    InstData(std::uint64_t val)
        : Int(val) { }
    InstData(double val)
        : Float(val) { }
    InstData(Index val)
        : Str(val) { }
    InstData(data::StrTok val)
        : StrTok(val) { }
    InstData(data::LineColumn val)
        : DbgStmt(val) { }
    InstData(data::Bin val)
        : Bin(val) { }
    InstData(RefInst val)
        : Ref(val) { }

    static inline InstData CreateNodePl(Index node, RefInst payload) {
        return InstData(data::NodePl { node, payload });
    }
    static inline InstData CreateTokPl(Index node, RefInst payload) { return InstData(data::TokPl { node, payload }); }
    static inline InstData CreateRef(RefInst value) { return { value }; }
    static inline InstData CreateBin(RefInst lhs, RefInst rhs) { return InstData(data::Bin { lhs, rhs }); }
    static inline InstData CreateStrTok(Index name, Index tok) { return InstData(data::StrTok { name, tok }); }
    static inline InstData CreateInt(std::uint64_t value) { return { value }; }
    static inline InstData CreateFloat(double value) { return { value }; }
};

ASSERT_8BYTES(InstData);

}

#endif
