set(TERM_SOURCES "terminal.cpp" "globals.cpp")

add_library(term_lib STATIC ${TERM_SOURCES})
target_compiler_settings(term_lib)

target_sources(${PROJECT_NAME} PRIVATE ${TERM_SOURCES})
