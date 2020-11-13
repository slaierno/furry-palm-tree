project(fpt-vm)

build_proj(LIBS fpt-libs GTEST PTHREAD)
target_precompile_headers(fpt-vm-includes INTERFACE "src/memory.hpp" "src/lc3-hw.hpp")
