set(TOKENIZER_SOURCES "Tokenizer.cpp" "TokenList.cpp")
set(PARSER_SOURCES "Parser.cpp" "Node.cpp" "parser/statements.cpp"
                   "parser/expressions.cpp")

set(AST_SOURCES ${TOKENIZER_SOURCES} ${PARSER_SOURCES})

add_library(ast_lib STATIC ${AST_SOURCES})
target_compiler_settings(ast_lib)
target_link_libraries(ast_lib logger_lib term_lib)

target_sources(${PROJECT_NAME} PRIVATE ${AST_SOURCES})
