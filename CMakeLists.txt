cmake_minimum_required(VERSION 3.0.0)
project(furry_palm_tree VERSION 0.1.0 LANGUAGES CXX)
include(CTest)
include(GoogleTest)

include(cmake/build.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Set compilers' warning level
if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -Werror)
endif()

set(DATA_DIR "\"${CMAKE_SOURCE_DIR}/data\"")
project(fpt)
add_subdirectory(libs)
add_subdirectory(fpt)
add_subdirectory(fpt-asm)
add_subdirectory(fpt-asm_v2)
add_subdirectory(external/googletest)