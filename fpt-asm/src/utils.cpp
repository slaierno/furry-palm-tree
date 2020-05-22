#include <algorithm>
#include <fstream>
#include "utils.hpp"
#include "errors.hpp"
#include "validationSM.hpp"
#include "builder.hpp"


/*********************************/
/*       CONSTS/TABLES           */
/*********************************/

//TODO inst_address should be somehow protected
uint16_t  inst_address = 0;
uint16_t start_address = 0x3000;
const std::string del = " ,\n\r\t";
std::map<std::string, uint16_t> label_map;


/*********************************/
/*      BINARY FILE UTILS        */
/*********************************/

void writeWords(std::ofstream& outfile, const uint16_t word, const size_t size) {
    uint16_t big_endian = word >> 8 | word << 8;
    outfile.write(reinterpret_cast<char const *>(&big_endian), 2 * size);
}

void writeMachineCode(std::ofstream& outfile, std::string line) {
    std::string ret_string = "";

    const uint16_t prev_address = inst_address;
    const uint16_t code = assembleLine(line, ret_string);
    if (!ret_string.empty()) {
        for(unsigned char const c : ret_string)
            writeWords(outfile, c);
        writeWords(outfile);
    } else {
        writeWords(outfile, code, inst_address - prev_address);
    }
}


/*********************************/
/*     ASSEMBLER FUNCTIONS       */
/*********************************/

void checkAddress(const uint16_t address) {
    if(address < userSpaceLower || address > userSpaceUpper)
        throw asm_error::out_of_user_space(address);
}
void updateInstructionAddress(unsigned int n) {
    inst_address+=n;
    checkAddress(inst_address);
}

/* Remove comments and heading whitespaces */
std::string cleanupLine(std::string line) {
    size_t first_valid = std::min(line.find_first_not_of(del), line.length());
    return line.substr(first_valid, line.find_first_of(";") - first_valid);
}

TokenList tokenize(std::string line) {
    line = cleanupLine(line);
    TokenList token_list; 
    /* Extract tokens */
    /* Dark magic happens here */
    /* Yes, I had fun. */
    auto l = [&] (const auto& self) -> int {
        return (line.length() && (
            token_list.push_back(line.substr(0, line.find_first_of(del))), 
            (line.erase(0, line.find_first_not_of(del, line.find_first_of(del))).length() && self(self))
        )); 
    }; l(l);
    return token_list;
}

void validateLine(const std::string& line) {
    TokenList token_list(tokenize(line));
    validateLineFSM(token_list);
}

uint16_t assembleLine(std::string& line, std::string& ret_string) {
    return assembleLine(tokenize(line), ret_string);
}

uint16_t assembleLine(std::string& line) {
    std::string ret_string = "";
    return assembleLine(tokenize(line), ret_string);
}
