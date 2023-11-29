include_guard(GLOBAL)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(
    FATAL_ERROR
      "CMake generation for 'koolang' is not allowed within the source directory!\n"
      "Please create a separate directory (e.g., 'build') and run CMake from there to keep your source directory clean and organized."
  )
endif()

# ----------------------------------------------------------------------------
# Set options
option(ENABLE_KOOLANG_TESTING "Build and run tests")

# -----------------------------------------------------------------------------
# Set policy
cmake_policy(SET CMP0076 NEW)

# Use folders for source file organization with IDE generators (Visual
# Studio/Xcode)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# -----------------------------------------------------------------------------
# Set compiler options

if(NOT DEFINED CMAKE_COMPILE_WARNING_AS_ERROR)
    set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
endif()

set(compiler_id "${CMAKE_CXX_COMPILER_ID}")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    include(${PROJECT_SOURCE_DIR}/cmake/compiler/clang_gcc.cmake)

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    include(${PROJECT_SOURCE_DIR}/cmake/compiler/clang_gcc.cmake)

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    include(${PROJECT_SOURCE_DIR}/cmake/compiler/msvc.cmake)
endif()

function(target_compiler_settings target)

    target_include_directories(${target} PRIVATE ${PROJECT_SOURCE_DIR}/src)
    # C++ standard
    target_compile_features(${target} PRIVATE cxx_std_23)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)

    target_compile_definitions(
    ${target}
    PRIVATE $<$<CONFIG:Debug>:KOOLANG_DEBUG_MODE>
            $<$<CONFIG:RelWithDebugInfo>:KOOLANG_DEBUG_MODE> VERSION="1.0")

    target_compile_options(
    ${target}
    PRIVATE $<$<CONFIG:Debug,RelWithDebugInfo>:${compiler_flags_debug}>
            ${compiler_flags})

endfunction()
