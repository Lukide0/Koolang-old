astgen_sources = files('AstGen.cpp', 'Scope.cpp', 'Printer.cpp')

astgen_lib = static_library('parser', astgen_sources,
    include_directories : inc,
    link_with : [parser_lib]
)

libs += astgen_lib
