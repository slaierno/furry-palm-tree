#include <iostream>
#include <fstream>
#include <string>
#include "assembler.hpp"

int main(int argc, const char* argv[])
{
    if (argc != 2)
    {
        /* show usage string */
        std::cout << "fpt-asm{.exe} asm-file" << std::endl;
        exit(2);
    }

    for (int j = 1; j < argc; ++j)
    {
        std::string in_filename = argv[j];
        std::string basename = in_filename.substr(0, in_filename.find_last_of('.'));
        std::string out_filename = basename + ".out";
        std::string dbg_filename = basename + ".dbg";
        assemble(in_filename, out_filename, dbg_filename);
    }

    return 0;
}
