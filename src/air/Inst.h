#ifndef KOOLANG_AIR_INST_H
#define KOOLANG_AIR_INST_H

#include "air/InstData.h"
#include "util/Index.h"
#include <unordered_map>
#include <vector>
namespace air {

enum class InstType : std::uint8_t {
    /// DECLARATIONS ///////////////////////////////////////////////////////////////////////////////

    /*
    Compile-time known value/type.
    Uses field 'PoolIndex'.
    */
    CONSTANT,

    /*
    Compile-time resolved symbol
    Uses field 'PoolIndex'.
    */
    SYMBOL,

    /*
    Compile-time not known value
    */
    // VARIABLE,

    /*
    Function
    */
    // FUNCTION,

    // ARG,

    // return by ref
    // RET_PTR,

    /// MEMORY ////////////////////////////////////////////////////////////////////////////////////

    /*
    Loads the value from the pointer.
    Uses field 'TyOp'
    */
    LOAD,

    // ALLOC,
    // TMP instruction
    // INFERRED_ALLOC,

    // Uses field 'TyOp'.
    CAST,

    /// BIN OP ////////////////////////////////////////////////////////////////////////////////////

    // Uses field 'BinOp'
    ADD,

    // Uses field 'BinOp'
    SUB,

    // Uses field 'BinOp'
    MUL,

    // Uses field 'BinOp'
    DIV,

    // Uses field 'BinOp'
    MOD,

    // PTR_ADD,
    // PTR_SUB,
    //
    // Uses field 'BinOp'
    BIT_AND,
    // Uses field 'BinOp'
    BIT_OR,
    // Uses field 'BinOp'
    BIT_SHL,
    // Uses field 'BinOp'
    BIT_SHR,
    // Uses field 'BinOp'
    BIT_XOR,
    // BIT_NEG,

    /// SPECIAL OP ////////////////////////////////////////////////////////////////////////////////

    // CALL,
    // TODO: attributes
    // CALL_INLINE,

};

struct Air {
    std::vector<InstData> Inst;
    std::vector<InstType> Type;
};

static_assert(sizeof(InstType) == 1, "Enum InstType must have size 1 byte");

}

#endif
