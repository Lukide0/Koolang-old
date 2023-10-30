set(AIR_SOURCES
    "Pool.cpp"
    "PoolKey.cpp"
    "ModuleManager.cpp"
    "symbol/SymbolMap.cpp"
    "type.cpp"
    "value.cpp"
    "Printer.cpp"
    "Sema.cpp"
    "sema/kirAs.cpp"
    "sema/globalDecl.cpp"
    "sema/kirBreak.cpp"
    "sema/kirDeclRef.cpp")

target_sources(${PROJECT_NAME} PRIVATE ${AIR_SOURCES})
