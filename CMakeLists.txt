cmake_minimum_required(VERSION 3.21)

project(
  koolang
  VERSION 0.1.0
  LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/bootstrap.cmake)

add_executable(${PROJECT_NAME})

# C++ standard
target_compiler_settings(${PROJECT_NAME})
add_subdirectory(src)

# -----------------------------------------------------------------------------
# Testing
if(ENABLE_KOOLANG_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
