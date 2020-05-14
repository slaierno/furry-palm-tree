cmake_minimum_required(VERSION 3.0.0)
project(furry_palm_tree VERSION 0.1.0 LANGUAGES CXX)

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

set(LIBS_DIR "libs")
set(SRC_DIR "src")
set(TEST_DIR "test")
set(EXT_DIR "external")
# Set variable with every header file
file(GLOB HEADER_FILES 
    "${SRC_DIR}/*/*.hpp"
    "${LIBS_DIR}/*/*.hpp")
# Set variable with every cpp file except the main ones
file(GLOB FPT_FILES "${SRC_DIR}/fpt/*.cpp")
list(FILTER FPT_FILES EXCLUDE REGEX ".*main.cpp$")

# Set executables' names and set their cpp files
set(MAIN_EXE furry_palm_tree)
set(TEST_EXE furry_palm_tree_test)
add_executable(${MAIN_EXE} "${SRC_DIR}/fpt/main.cpp")
add_executable(${TEST_EXE} "${TEST_DIR}/fpt/main_test.cpp")

# Set the project as a library linked by the main files
add_library(FPT_LIB ${FPT_FILES})
target_link_libraries(${MAIN_EXE} FPT_LIB)
target_link_libraries(${TEST_EXE} FPT_LIB)

# Set pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${MAIN_EXE} Threads::Threads)
target_link_libraries(${TEST_EXE} Threads::Threads)

# Set gtest
# TODO gtest can be even built via CMake
link_directories("external/googletest/mybuild/lib")
target_link_libraries(${TEST_EXE} gtest)
include(GoogleTest)
enable_testing()
gtest_discover_tests(${TEST_EXE})
