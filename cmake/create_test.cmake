function(create_test name)
    set(prefix ARG)
    set(multiple_value_options FILES)

    cmake_parse_arguments(PARSE_ARGV 2 ${prefix} "" ""
                        "${multiple_value_options}")

    add_executable("${name}_test" ${ARG_FILES})
    add_test(NAME ${name} COMMAND "${name}_test")

endfunction()
