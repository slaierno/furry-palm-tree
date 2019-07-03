#include <iostream>
#include <fstream>
#include "Token.hpp"
#include "utils.hpp"

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        /* show usage string */
        std::cout << "lc3-asm{.exe} [image-file1] ..." << std::endl;
        exit(2);
    }

    for (int j = 1; j < argc; ++j)
    {
        auto filename = argv[j];
        std::ifstream asm_file;
        asm_file.open(filename);
        if (asm_file.is_open()) {
            std::ofstream outfile;
            outfile.open("junk.obj", std::ios::binary | std::ios::out);
            std::string line;
            while (getline(asm_file, line)) {
                validateLine(line);
            }
            writeWords(outfile, start_address);
            inst_address = start_address;
            asm_file.clear();
            asm_file.seekg(std::ios::beg);
            while (getline(asm_file, line)) {
                writeMachineCode(outfile, line);
            }
            outfile.close();
            asm_file.close();
        }
    }
}
