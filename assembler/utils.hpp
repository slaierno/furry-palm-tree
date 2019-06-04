#pragma once

#include <string>
#include <vector>
#include "Token.hpp"
#include "commons.hpp"

extern uint16_t (*inst_table[OP::COUNT])(const TokenList&);
extern uint16_t inst_address;
extern std::map<std::string, uint16_t> label_map;

TokenList   tokenize(std::string line);
void        validationStep(TokenList tokens);
void        validateLine(std::string&);
uint16_t    assembleLine(std::string&);
std::string typeToString(enum TokenType const);
std::string tokenToString(Token const&);
std::string typeToShortString(enum TokenType const);
std::string tokenToShortString(Token const&);
void        checkBitRange(Token const&, const int nBit);
void        checkBitRangeUnsigned(Token const&, const int nBit);