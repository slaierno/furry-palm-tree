cmake_minimum_required(VERSION 3.0.0)
project(furry_palm_tree VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INCLUDE_DIR "include")
# Set variable with every header file
set(HEADER_FILES 
"${INCLUDE_DIR}/LabelMap.hpp"
"${INCLUDE_DIR}/DebugSymbols.hpp"
"${INCLUDE_DIR}/cx/cx_algorithm.hpp"
"${INCLUDE_DIR}/cx/cx_map.hpp"
"${INCLUDE_DIR}/cx/cx_string.hpp"
"${INCLUDE_DIR}/cx/cx_vector.hpp"
"${INCLUDE_DIR}/cx/cx.hpp"
"lc3-hw.hpp"
"lc3-debug.hpp"
"lc3-video.hpp"
"memory.hpp"
"InputBuffer.hpp"
)
# Set variable with every cpp file except the main ones
set(SRC_FILES
"lc3-hw.cpp"
"memory.cpp"
)

# Set executables' names and set their cpp files
set(MAIN_EXE furry_palm_tree)
set(TEST_EXE furry_palm_tree_test)
add_executable(${MAIN_EXE} "main.cpp")
add_executable(${TEST_EXE} "main_test.cpp")

# Set the project as a library linked by the main files
add_library(SRC_LIB ${SRC_FILES})
target_link_libraries(${MAIN_EXE} SRC_LIB)
target_link_libraries(${TEST_EXE} SRC_LIB)

# Set pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${MAIN_EXE} Threads::Threads)
target_link_libraries(${TEST_EXE} Threads::Threads)

# Set gtest
# TODO gtest can be even built via CMake
link_directories("googletest/lib")
target_link_libraries(${TEST_EXE} gtest)
include(GoogleTest)
enable_testing()
gtest_discover_tests(${TEST_EXE})
