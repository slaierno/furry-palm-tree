/* This header should be used by gtest only and gives access
 * to otherwise "private" functions
 */
#pragma once

#include "assembler.hpp"

bool consume_labels(Instruction& inst, LabelMap& label_map, uint16_t address = 0);

void assemble_step1(std::istream& asm_file, Program& program);
void assemble_step2(const Program& program, LabelMap& label_map);
void assemble_step3(Program& program, LabelMap& label_map);
void assemble_step4(const Program& program, LabelMap& label_map, 
                    std::ofstream& out_file, std::ofstream& dbg_file);