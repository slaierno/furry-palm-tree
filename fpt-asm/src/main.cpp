#include <iostream>
#include <fstream>
#include "Token.hpp"
#include "utils.hpp"

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        /* show usage string */
        std::cout << "fpt-asm{.exe} [asm-file1] [-o obj-file1] ..." << std::endl;
        exit(2);
    }

    for (int j = 1; j < argc; ++j)
    {
        std::string in_filename = argv[j];
        std::string out_filename;
        if(argc >= j+2 && std::string(argv[j+1]).compare("-o") == 0) {
            out_filename = argv[j+2];
            j+=2;
        } else {
            out_filename = in_filename.substr(0, in_filename.find_last_of('.')) + ".obj";
        }
        std::ifstream asm_file;
        asm_file.open(in_filename);
        if (asm_file.is_open()) {
            std::ofstream outfile;
            std::string basename = in_filename.substr(0, in_filename.find_last_of('.'));
            outfile.open(out_filename, std::ios::binary | std::ios::out);
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
