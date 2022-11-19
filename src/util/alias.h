#ifndef KOOLANG_UTIL_H
#define KOOLANG_UTIL_H

constexpr int TOKEN_BUFF_SIZE = 1 << 10;
constexpr int AST_BUFF_SIZE   = 1 << 9;

#define DISCARD_VALUE(EXPR) static_cast<void>(EXPR)
#define ASSERT_8BYTES(TYPE) static_assert(sizeof(TYPE) <= 8, "Size of the '" #TYPE "' is greater than 8bytes")

#endif // KOOLANG_UTIL_H
