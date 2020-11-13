project(fpt-asm_v2)

build_proj(LIBS fpt-libs fpt-vm-includes GTEST PTHREAD)
target_precompile_headers(fpt-asm_v2 PRIVATE "src/commons.hpp")
