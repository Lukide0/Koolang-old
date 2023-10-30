set(LOGGER_SOURCES "Label.cpp" "Record.cpp" "colors.cpp")

add_library(logger_lib STATIC ${LOGGER_SOURCES})
target_compiler_settings(logger_lib)

target_sources(${PROJECT_NAME} PRIVATE ${LOGGER_SOURCES})
