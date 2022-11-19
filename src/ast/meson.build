tokenizer_sources = files('Tokenizer.cpp', 'TokenList.cpp')
parser_sources = files('Parser.cpp', 'Node.cpp', 'parser/statements.cpp', 'parser/expressions.cpp')

tokenizer_lib = static_library('tokenizer', tokenizer_sources,
    include_directories : inc,
    link_with : [logger_lib]
)

parser_lib = static_library('parser', parser_sources,
    include_directories : inc,
    link_with : [tokenizer_lib, term_lib]
)

libs += parser_lib
