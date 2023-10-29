include_guard(GLOBAL)

set(compiler_flags
    /W4
    /permissive-
    /Zc:inline
    /Zc:preprocessor
    /Zc:enumTypes
    /Zc:externConstexpr
    /Zc:lambda
    /EHsc
    /w14165
    /w44242
    /w44254
    /w44263
    /w34265
    /w34287
    /w44296
    /w44365
    /w44388
    /w44464
    /w14545
    /w14546
    /w14547
    /w14549
    /w14555
    /w34619
    /w14928
    /w45038)

set(compiler_flags_debug /fsanitize=address)
