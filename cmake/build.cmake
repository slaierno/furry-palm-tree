function(build_proj)
    cmake_parse_arguments(BUILD "PTHREAD;GTEST"
                                "" 
                                "LIBS" 
                                ${ARGN})
                                
    set(MAIN_EXE ${PROJECT_NAME})
    set(TEST_EXE ${PROJECT_NAME}_test)
    add_executable(${MAIN_EXE} src/main.cpp)
    add_executable(${TEST_EXE} src test/main_test.cpp)

    file(GLOB SRC_FILES "src/*.cpp")
    file(GLOB HEAD_FILES "src/*.hpp")
    list(FILTER SRC_FILES EXCLUDE REGEX ".*main.cpp$")
    # add_library(SRC_LIB ${SRC_FILES})

    target_sources(${MAIN_EXE} PRIVATE ${SRC_FILES})
    target_sources(${TEST_EXE} PRIVATE ${SRC_FILES})
    target_compile_definitions(${MAIN_EXE} PUBLIC -DFPT_DATA_DIR="\"${DATA_DIR}\"")
    target_compile_definitions(${TEST_EXE} PUBLIC -DFPT_DATA_DIR="\"${DATA_DIR}\"")
        
    target_link_libraries(${MAIN_EXE} ${BUILD_LIBS})
    add_library("${MAIN_EXE}-includes" INTERFACE)
    target_include_directories("${MAIN_EXE}-includes" INTERFACE "src")
    target_include_directories(${TEST_EXE} PRIVATE "src")
    target_link_libraries(${TEST_EXE} ${BUILD_LIBS} gtest)

    if(${BUILD_PTHREAD})
        # Set pthread
        set(THREADS_PREFER_PTHREAD_FLAG ON)
        find_package(Threads REQUIRED)
        target_link_libraries(${MAIN_EXE} Threads::Threads)
        target_link_libraries(${TEST_EXE} Threads::Threads)
    endif()

    if(${BUILD_GTEST})
        # Set gtest
        enable_testing()
        gtest_discover_tests(${TEST_EXE})
    endif()
endfunction()