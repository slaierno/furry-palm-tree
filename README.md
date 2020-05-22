# furry-palm-tree
Furry Palm Tree (FPT) is a VM for a LC3-based computer with video hardware. It is intended for creation of little old-school games and it is meant to be a self-educational project.

## Building

Requirements:
* A cutting-edge C++20 compiler.
* CMake >= 3.0 or newer.
* A version of `make` (MSVC has not been tested yet, but it *should* work with minimal effort).

How to build:

* `mkdir build`
* [optional if C++20 compiler is not the system default one]  
`export CC=<your-c++20-compiler-path>`
* `cmake ..`
* `cmake --build . -j`
* [optional]  
`ctest`

The binaries can be found in `build/[fpt, fpt-asm, fpt-asm_v2]
