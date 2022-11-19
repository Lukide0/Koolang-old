#include "Node.h"
#include <string_view>

using namespace std::literals::string_view_literals;

namespace ast {

std::string_view tagToStr(Tag tag) {
    switch (tag) {
    case Tag::ROOT:
        return "ROOT"sv;
    case Tag::BLOCK:
        return "BLOCK"sv;
    case Tag::IMPORT:
        return "IMPORT"sv;
    case Tag::IMPORT_PATH:
        return "IMPORT_PATH"sv;
    case Tag::CONSTANT:
        return "CONSTANT"sv;
    case Tag::VARIABLE:
        return "VARIABLE"sv;
    case Tag::STATIC:
        return "STATIC"sv;
    case Tag::FN:
        return "FN"sv;
    case Tag::FN_DEF:
        return "FN_DEF"sv;
    case Tag::FN_PARAMS:
        return "FN_PARAMS"sv;
    case Tag::FN_PARAM:
        return "FN_PARAM"sv;
    case Tag::VARIANT:
        return "VARIANT"sv;
    case Tag::IMPL:
        return "IMPL"sv;
    case Tag::IMPL_BODY:
        return "IMPL_BODY"sv;
    case Tag::STRUCT:
        return "STRUCT"sv;
    case Tag::STRUCT_FIELD:
        return "STRUCT_FIELD"sv;
    case Tag::STRUCT_CONST:
        return "STRUCT_CONST"sv;
    case Tag::ENUM:
        return "ENUM"sv;
    case Tag::ENUM_FIELD:
        return "ENUM_FIELD"sv;
    case Tag::IF_STMT:
        return "IF_STMT"sv;
    case Tag::FOR_STMT:
        return "FOR_STMT"sv;
    case Tag::WHILE_STMT:
        return "WHILE_STMT"sv;
    case Tag::TRAIT:
        return "TRAIT"sv;
    case Tag::PATH:
        return "PATH"sv;
    case Tag::LITERAL:
        return "LITERAL"sv;
    case Tag::ARRAY:
        return "ARRAY"sv;
    case Tag::ARRAY_SHORT:
        return "ARRAY_SHORT"sv;
    case Tag::TUPLE:
        return "TUPLE"sv;
    case Tag::DISCARD:
        return "DISCARD"sv;
    case Tag::GROUPED_EXPR:
        return "GROUPED_EXPR"sv;
    case Tag::CAST_EXPR:
        return "CAST_EXPR"sv;
    case Tag::CLOSURE_EXPR:
        return "CLOSURE_EXPR"sv;
    case Tag::CLOSURE_CAPTURES:
        return "CLOSURE_CAPTURES"sv;
    case Tag::CLOSURE_CAPTURE:
        return "CLOSURE_CAPTURE"sv;
    case Tag::TYPE:
        return "TYPE"sv;
    case Tag::TYPE_ARR:
        return "TYPE_ARR"sv;
    case Tag::TYPE_TUPLE:
        return "TYPE_TUPLE"sv;
    case Tag::TYPE_DYNAMIC:
        return "TYPE_DYNAMIC"sv;
    case Tag::PATTERN_DISCARD:
        return "PATTERN_DISCARD"sv;
    case Tag::PATTERN_SINGLE:
        return "PATTERN_SINGLE"sv;
    case Tag::PATTERN_MULTIPLE:
        return "PATTERN_MULTIPLE"sv;
    case Tag::PATTERN_STRUCT:
        return "PATTERN_STRUCT"sv;
    case Tag::PATTERN_STRUCT_FIELD:
        return "PATTERN_STRUCT_FIELD"sv;
    case Tag::SINGLE_OP:
        return "SINGLE_OP"sv;
    case Tag::UNWRAP_OP:
        return "UNWRAP_OP"sv;
    case Tag::BIN_OP:
        return "BIN_OP"sv;
    case Tag::SLICE_OP:
        return "SLICE_OP"sv;
    case Tag::CALL_OP:
        return "CALL_OP"sv;
    case Tag::FLOW_OP:
        return "FLOW_OP"sv;
    case Tag::STRUCT_EXPR:
        return "STRUCT_EXPR"sv;
    case Tag::STRUCT_EXPR_FIELDS:
        return "STRUCT_EXPR_FIELDS"sv;
    case Tag::STRUCT_EXPR_FIELD:
        return "STRUCT_EXPR_FIELD"sv;
    case Tag::TYPE_FN:
        return "TYPE_FN"sv;
    default:
        return ""sv;
    }
}

}
