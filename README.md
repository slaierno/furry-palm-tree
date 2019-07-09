# furry-palm-tree
Furry Palm Tree (FPT) is a VM for a LC3-based computer with video hardware. It is intended for creation of little old-school games and it is meant to be a self-educational project.

## Building

Requirements:
* Any suitable C++17 compiler.
* A version of `make`.
* [Google Test](https://github.com/google/googletest)
* `wxWidgets-3.0.4-2` or later
    * MSYS2: `pacman -S mingw-w64-x86_64-wxWidgets`
    * WSL?
    * Linux?

How to build:

* `make release` build an executable capable of running the VM.
* `make test` run the unit tests
* `cd assembler && make all` build the assembler
* `cd assembler && make test` test the assembler
* Append `CC=<your-favourite-compiler>` if you do not want to use `gcc`
