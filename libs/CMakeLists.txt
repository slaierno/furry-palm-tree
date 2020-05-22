project(fpt-libs)

add_library(fpt-libs INTERFACE)

file(GLOB_RECURSE INCLUDE_FILES *.hpp)
target_sources(${PROJECT_NAME} INTERFACE ${INCLUDE_FILES})

target_include_directories(fpt-libs
    INTERFACE
        ${PROJECT_SOURCE_DIR}/include/
        ${PROJECT_SOURCE_DIR}/
        )