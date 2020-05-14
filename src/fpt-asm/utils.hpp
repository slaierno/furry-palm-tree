#pragma once

#include <string>
#include <vector>
#include "Token.hpp"
#include "commons.hpp"

/*********************************/
/*       CONSTS/TABLES           */
/*********************************/
extern uint16_t (*inst_table[OP::COUNT])(const TokenList&);
extern uint16_t  inst_address;
extern uint16_t start_address;
extern std::map<std::string, uint16_t> label_map;


/*********************************/
/*      BINARY FILE UTILS        */
/*********************************/
void writeWords(std::ofstream& outfile, const uint16_t word = 0, const size_t size = 1);
void writeMachineCode(std::ofstream& outfile, std::string line);


/*********************************/
/*     ASSEMBLER FUNCTIONS       */
/*********************************/
void        updateInstructionAddress(unsigned int n = 1);
TokenList   tokenize(std::string line);
void        validateLine(const std::string&);
uint16_t    assembleLine(std::string&, std::string& ret_string);
uint16_t    assembleLine(std::string&); //discards return string