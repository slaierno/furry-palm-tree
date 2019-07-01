#pragma once

#include <string>
#include <vector>
#include "Token.hpp"
#include "commons.hpp"

extern uint16_t (*inst_table[OP::COUNT])(const TokenList&);
extern uint16_t  inst_address;
extern uint16_t start_address;
extern std::map<std::string, uint16_t> label_map;

void        updateInstructionAddress(unsigned int n = 1);
TokenList   tokenize(std::string line);
void        validateLine(const std::string&);
uint16_t    assembleLine(std::string&, std::string& ret_string);
uint16_t    assembleLine(std::string&); //discards return string