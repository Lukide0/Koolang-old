function(create_test name)
    set(prefix ARG)
    set(multiple_value_options FILES INCLUDE LIBS)

    cmake_parse_arguments(PARSE_ARGV 1 ${prefix} "" ""
                        "${multiple_value_options}")

    add_executable(${name} ${ARG_FILES})

    target_compiler_settings(${name})
    target_include_directories(${name} PRIVATE ${ARG_INCLUDE})
    target_link_libraries(${name} ${ARG_LIBS})

    add_test(NAME ${name} COMMAND ${name})
endfunction()
