#ifndef KOOLANG_KIR_REFINST_H
#define KOOLANG_KIR_REFINST_H

#include "util/Index.h"
namespace kir {

struct RefInst {
    enum Constants : Index {
        NONE = bitFlag(0),

        ZERO       = bitFlag(1),
        ONE        = bitFlag(2),
        NULL_VALUE = bitFlag(3),
        BOOL_TRUE  = bitFlag(4),
        BOOL_FALSE = bitFlag(5),

        VOID_TYPE  = bitFlag(6),
        BOOL_TYPE  = bitFlag(7),
        U8_TYPE    = bitFlag(8),
        I8_TYPE    = bitFlag(9),
        U16_TYPE   = bitFlag(10),
        I16_TYPE   = bitFlag(11),
        U32_TYPE   = bitFlag(12),
        I32_TYPE   = bitFlag(13),
        U64_TYPE   = bitFlag(14),
        I64_TYPE   = bitFlag(15),
        USIZE_TYPE = bitFlag(16),
        ISIZE_TYPE = bitFlag(17),
        F16_TYPE   = bitFlag(18),
        F32_TYPE   = bitFlag(19),
        F64_TYPE   = bitFlag(20),
        STR_TYPE   = bitFlag(21),
        CHAR_TYPE  = bitFlag(22),
    };

    // highest bit
    static constexpr Index REF_CONST_BIT  = 1U << (INDEX_BITS - 1);
    static constexpr Index REF_VALUES     = ZERO | ONE | NULL_VALUE | BOOL_TRUE | BOOL_FALSE;
    static constexpr Index REF_TEST_VALUE = REF_CONST_BIT | REF_VALUES;

    RefInst(Index inst)
        : Offset(inst) { }

    RefInst(Constants constant)
        : Offset(constant | REF_CONST_BIT) { }

    [[nodiscard]] Constants ToConstant() const { return static_cast<Constants>(Offset & (~REF_CONST_BIT)); }

    // The Payload variable represents either a Constant or value specified by the instruction, depending on whether
    // the highest bit is set. If the bit is set then the value is Constant.
    Index Offset;

    static bool IsConstant(Index inst) { return (inst & REF_CONST_BIT) != 0; }
    [[nodiscard]] bool IsConstant() const { return (Offset & REF_CONST_BIT) != 0; }
    [[nodiscard]] bool IsValue() const { return (Offset & REF_TEST_VALUE) > REF_CONST_BIT; }
};

}

#endif
