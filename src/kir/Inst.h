#ifndef KOOLANG_KIR_INST_H
#define KOOLANG_KIR_INST_H

#include "kir/InstData.h"
#include "util/Index.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace kir {

enum class InstType : std::uint8_t {
    NONE,

    /// DECLARATIONS //////////////////////////////////////////////////////////////////////////////

    /* Stores the index to the Strings */
    IDENT,

    /*
     * Defines global item
     *
     * Uses field 'Bin'. Lhs is 'Decl'. Rhs is 'BLOCK_INLINE' instruction.
     */
    DECL,

    /*
     * Defines function
     *
     * Uses field 'Bin'. Lhs is 'DeclFn'. Rhs is 'BLOCK' instruction.
     */
    DECL_FN,
    /*
     * Defines enum
     *
     * Uses field 'Bin'. Lhs is 'DeclEnum'. Rhs is 'BLOCK_COMPTIME_INLINE' instruction.
     */
    DECL_ENUM,

    /*
     * Defines struct
     * Uses field 'Bin'. Lhs is 'DeclStruct'. Rhs is 'BLOCK_COMPTIME_INLINE' instruction.
     */
    DECL_STRUCT,
    DECL_VARIANT,

    /*
     * Defines struct field
     *
     */
    STRUCT_FIELD,

    /*
     * Uses name to identify symbol.
     *
     * Uses field 'TokPl'. Payload is name index.
     */
    DECL_REF,

    /*
     * Same as DECL_REF, except this symbol is in expression path
     *
     * Uses field 'TokPl'. Payload is DeclItem
     *
     */
    DECL_ITEM,

    /*
     * Function param
     *
     * Uses field 'NodePl'. Payload is Param.
     */
    PARAM,

    /*
     * Enum field
     *
     * Uses field 'NodePl'. Payload is 'DeclEnumField'.
     */
    ENUM_FIELD,

    /*
     * Access field or namespace.
     *
     * Uses field 'NodePl'. Payload is '{ count, trailing identifiers }'
     */
    NAMESPACE,

    /// BLOCKS ////////////////////////////////////////////////////////////////////////////////////

    /*
     * Block
     *
     * Uses field 'NodePl'. Payload is 'Block'
     */
    BLOCK,
    LOOP,

    /*
     * A list of instructions which are analyzed in the parent context, without generating a runtime block.
     *
     * Uses field 'NodePl'. Payload is 'Block'.
     */
    BLOCK_INLINE,

    /* Same as BLOCK_INLINE */
    BLOCK_COMPTIME_INLINE,

    /*
     * Returns a value from BLOCK_INLINE.
     *
     * Uses field 'Bin'. Lhs points to 'BLOCK_INLINE' or 'BLOCK' and Rhs to return value.
     */
    BREAK_INLINE,

    /*
     * Returns a value from BLOCK.
     *
     * Uses field 'NodePl'. Payload is label name.
     */
    BREAK,

    // Same as BREAK except the value is returned from function
    RETURN,

    /*
     * Skips the rest of the loop.
     *
     * Uses field 'NodePl'. Payload is label name.
     */
    CONTINUE,

    /*
     * Logical AND.
     *
     * Uses field 'Bin'. Rhs points to 'BLOCK_INLINE' or 'BLOCK'.
     */

    LOGIC_AND,
    /*
     * Logical OR.
     *
     * Uses field 'Bin'. Rhs points to 'BLOCK_INLINE' or 'BLOCK'.
     */
    LOGIC_OR,

    /*
     * Uses field 'Ref'
     */
    GOTO,
    REPEAT,

    /// MEMORY ////////////////////////////////////////////////////////////////////////////////////

    /*
     * Allocates stack local memory
     *
     * Uses field 'NodePl'. Payload is type instruction
     */
    ALLOC,
    ALLOC_MUT,

    /*
     * Allocates stack local memory(unknown type).
     *
     * Uses field 'Ref' which points to AST node.
     */
    ALLOC_INFERRED,
    ALLOC_MUT_INFERRED,

    /*
     * Writes value
     *
     * Uses field 'Bin'. Lhs is source, Rhs is value.
     */
    STORE,
    STORE_INFERRED,

    /*
     * Same as 'STORE' except provides a source location
     *
     * Uses field 'NodePl'. Payload is 'Bin'
     */
    STORE_NODE,

    /*
     * Loads value from instruction
     */
    LOAD,

    /*
     * Returns the len property. Used in for loop.
     *
     * Uses field 'Ref'.
     */
    INDEXABLE_LEN,

    /// CHECKS ////////////////////////////////////////////////////////////////////////////////////

    DISCARD_DESTRUCTOR,

    /// EXPRESSIONS ///////////////////////////////////////////////////////////////////////////////

    /*
     * Array initialization
     *
     * Uses field 'NodePl'. Payload is 'Block'
     */
    ARR_INIT,

    /*
     * Array short initialization
     *
     * Uses field 'NodePl'. Payload is 'ArrayShortInit'
     */
    ARR_SHORT_INIT,

    /*
     * Tuple value
     *
     * Uses field 'NodePl'. Payload is 'Block'
     */
    TUPLE,

    /*
     * Cast
     *
     * Uses field 'Bin'. Lhs is type. Rhs is value.
     */
    CAST,

    /*
     * Call expression
     *
     * Uses field 'NodePl'. Payload is 'Call'
     *
     */
    CALL,

    /*
     * Conditional branch
     *
     * Uses field 'Bin'. Lhs is condition, Rhs is 'IfData'.
     */
    CONDBR,

    /// LITERALS //////////////////////////////////////////////////////////////////////////////////

    /*
     * 64bit number literal
     *
     * Uses field 'Int'.
     */
    INT,
    /*
     * 64 float literal
     *
     * Uses field 'Float'
     */
    FLOAT,
    /*
     * String literal.
     *
     * Uses field 'StrTok'
     */
    STR,
    /*
     * Character literal.
     *
     * Uses field 'StrTok'
     */
    CHAR,

    /// BIN OP ////////////////////////////////////////////////////////////////////////////////////
    /// All instruction uses 'NodePl' with payload 'Bin'

    /*
     * Op: a + b
     */
    ADD,

    /*
     * Op: a - b
     */
    SUB,

    /*
     * Op: a * b
     */
    MUL,

    /*
     * Op: a / b
     */
    DIV,

    /*
     * Op: a % b
     */
    MOD,

    /*
     * Op: a[b]
     */
    ARR_EL,

    /*
     * Op: a.b
     */
    FIELD,

    /*
     * Op: a.b
     * Uses field 'Bin'.
     */
    FIELD_SHORT,

    /*
     * Op: a < b
     */
    CMP_LS,

    /*
     * Op: a > b
     */
    CMP_GT,

    /*
     * Op: a <= b
     */
    CMP_LSE,

    /*
     * Op: a >= b
     */
    CMP_GTE,

    /*
     * Op: a == b
     */
    CMP_EQ,

    /*
     * Op: a != b
     */
    CMP_NEQ,

    /*
     * Op: a & b
     */
    BIT_AND,
    /*
     * Op: a | b
     */
    BIT_OR,

    /*
     * Op: a << b
     */
    BIT_SHL,

    /*
     * Op: a >> b
     */
    BIT_SHR,

    /*
     * Op: a ^ b
     */
    BIT_XOR,

    /// SPECIAL OP ////////////////////////////////////////////////////////////////////////////////

    /*
     * Op: |[a;from,to]|
     * Uses field 'NodePl'. Payload is 'Slice'
     */
    SLICE_FULL,

    /*
     * Op: |[a;from]|
     * Uses field 'NodePl'. Payload is 'Bin'. Lhs is 'base'. Rhs is 'from'
     */
    SLICE_START,

    /*
     * Op: |[a;_,to]|
     * Uses field 'NodePl'. Payload is 'Bin'. Lhs is 'base'. Rhs is 'to'
     */
    SLICE_END,

    /*
     * Op: new A
     * Uses field 'NodePl'. Payload is path instruction
     */
    STRUCT_INIT_EMPTY,

    /*
     * Op: new A{}
     * Uses field 'NodePl'. Payload is 'StructInit'
     */
    STRUCT_INIT,

    /// SINGLE OP /////////////////////////////////////////////////////////////////////////////////
    /// All instruction uses 'NodePl'

    /*
     * Op: !a
     */
    BOOL_NEG,

    /*
     * Op: ~a
     */
    BIT_NEG,

    /*
     * Op: &a
     */
    GET_ADDR,

    /*
     * Op: *a
     */
    DEREF,

    /*
     * Op: -a
     */
    INT_NEG,

    /*
     * Op: a?
     */
    UNWRAP,

    /// TYPES /////////////////////////////////////////////////////////////////////////////////////

    /*
     * Type coercion
     *
     * Uses field 'Bin'. Lhs is type. Rhs is value.
     */
    AS,

    /*
     * Array type
     *
     * Uses field 'NodePl'. Payload is 'Bin'. Lhs is length, rhs is element type
     */
    ARRAY_TYPE,

    /*
     * Ptr type
     *
     * Uses field 'NodePl'. Payload is 'Bin'. Lhs is count of '*', rhs is element type
     */
    PTR_TYPE,

    /*
     * Tuple type
     *
     * Uses field 'NodePl'. Payload is 'Block'
     */
    TUPLE_TYPE,

    /*
     * Dynamic type
     *
     * Uses field 'NodePl'. Payload is 'Block'
     */
    DYN_TYPE,

    /*
     * Reference type
     *
     * Uses field 'NodePl'. Payload is element type
     */
    REF_TYPE,

    /*
     * Slice type
     *
     * Uses field 'NodePl'. Payload is element type
     */
    SLICE_TYPE,

    /// DEBUG INFORMATIONS ////////////////////////////////////////////////////////////////////////

};

struct Kir {
    std::vector<InstData> Inst;
    std::vector<InstType> Type;
    std::vector<Index> Extra;
    std::vector<std::string> Strings;
    std::vector<Index> Imports;
};

static_assert(sizeof(InstType) == 1, "Enum InstType must have size 1 byte");

}

#endif // KOOLANG_KIR_INST_H
