air_sources = files(
    'Pool.cpp',
    'PoolKey.cpp',
    'ModuleManager.cpp',
    'symbol/SymbolMap.cpp',
    'type.cpp',
    'value.cpp',
    'Printer.cpp',
    'Sema.cpp',
    'sema/kirAs.cpp',
    'sema/globalDecl.cpp',
    'sema/kirBreak.cpp',
    'sema/kirDeclRef.cpp',
)

air_lib = static_library('air', air_sources,
    include_directories : inc,
    link_with : [astgen_lib]
)

libs += air_lib
