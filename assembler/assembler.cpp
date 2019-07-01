#include <iostream>
#include <fstream>
#include "Token.hpp"
#include "utils.hpp"

void writeMachineCode(std::ofstream& outfile, std::string line) {
    auto writeBigEndian = [&outfile](uint16_t code = 0, size_t size = 1) constexpr {
        const uint16_t big_endian = code >> 8 | code << 8;
        outfile.write(reinterpret_cast<char const *>(&big_endian), 2 * size);
    };
    std::string ret_string = "";

    const uint16_t prev_address = inst_address;
    const uint16_t code = assembleLine(line, ret_string);
    if (!ret_string.empty()) {
        for(unsigned char const c : ret_string)
            writeBigEndian(c);
        writeBigEndian();
    } else {
        writeBigEndian(code, inst_address - prev_address);
    }
}

void writeAddress(std::ofstream&outfile, const uint16_t address) {
    uint16_t big_endian = address >> 8 | address << 8;
    outfile.write(reinterpret_cast<char const *>(&big_endian), 2);
}

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
            writeAddress(outfile, start_address);
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
