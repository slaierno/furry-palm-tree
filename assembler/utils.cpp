#include <algorithm>
#include "utils.hpp"
#include "errors.hpp"
#include "validationSM.hpp"
#include "builder.hpp"

//TODO inst_address should be somehow protected
uint16_t  inst_address = 0;
uint16_t start_address = 0x3000;
std::map<std::string, uint16_t> label_map;
const std::string del = " ,\n\r\t";

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
