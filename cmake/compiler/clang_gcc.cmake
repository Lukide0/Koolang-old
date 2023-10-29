include_guard(GLOBAL)

set(compiler_flags
    -Wall
    -Wextra
    -Wformat
    -Wpedantic
    -Wconversion
    -Wshadow
    -Wunused
    -Wsign-conversion
    -Wcast-qual
    -Wcast-align
    -Wdouble-promotion
    -Wimplicit-fallthrough
    -Wundef
    -Wfloat-equal
    -Wnull-dereference
    -Wextra-semi
    -Woverloaded-virtual
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wpessimizing-move
    -Wredundant-move
    -Wself-move)

set(compiler_flags_debug -fsanitize=address,undefined)
