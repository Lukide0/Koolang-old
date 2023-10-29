cmake_minimum_required(VERSION 3.21)

project(
  koolang
  VERSION 0.1.0
  LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/bootstrap.cmake)

add_executable(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# C++ standard
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE $<$<CONFIG:Debug>:KOOLANG_DEBUG_MODE>
          $<$<CONFIG:RelWithDebugInfo>:KOOLANG_DEBUG_MODE> VERSION="1.0")

target_compile_options(
  ${PROJECT_NAME}
  PRIVATE $<$<CONFIG:Debug,RelWithDebugInfo>:${compiler_flags_debug}>
          ${compiler_flags})

add_subdirectory(src)

# -----------------------------------------------------------------------------
# Testing
if(ENABLE_KOOLANG_TESTING)
  add_subdirectory(test)
endif()
