#pragma once

#include <deque>
#include "Token.hpp"
#include "Instruction.hpp"

// using Instruction = std::deque<Token>;
using Program = std::vector<Instruction>;

/* Checks against the TokenType(s) given via template arguments. The first token
    is skipped, given that it should be a TokenType::Instruction or a TokenType::PseudoOp
 */
template <TokenType ...types> bool check_arguments(const Instruction& inst) {
    if (sizeof...(types) != inst.size() - 1) return false;
    constexpr TokenType types_vec[] = {types...};
    for(size_t i = 1; i < inst.size(); i++)
        if (inst[i].getType() != types_vec[i-1]) return false;
    return true;
}

void assemble(const std::string& in_filename, const std::string& out_filename, const std::string& dbg_filename);