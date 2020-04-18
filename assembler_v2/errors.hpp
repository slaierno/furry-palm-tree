#pragma once
#include <string>
#include <sstream>
#include "Token.hpp"
#include "commons.hpp"

inline std::string error_string(const OP::Type op) {
    std::stringstream ss;
    ss << "Instruction " << op << " must be followed by ";
    switch(op) {
    case OP::ADD: case OP::AND: case OP::XOR:
        ss << "two registers and a register or an immediate value.";
        break;
    case OP::NOT:
        ss << "two registers.";
        break;
    case OP::JSRR: case OP::JMP:
        ss << "a register.";
        break;
    case OP::RET: case OP::RTI: case OP::GETC: case OP::OUT: 
    case OP::PUTS: case OP::IN: case OP::PUTSP: case OP::HALT:
        ss << "no arguments.";
        break;
    case OP::LDR: case OP::STR: case OP::LSHF: case OP::RSHFA: case OP::RSHFL:
        ss << "two registers and an immediate value";
        break;
    case OP::BR: case OP::BRn: case OP::BRz: case OP::BRp: case OP::BRnz:
    case OP::BRnp: case OP::BRzp: case OP::BRnzp: case OP::JSR:
        ss << "a label.";
        break;
    case OP::LD: case OP::LDI: case OP::LEA: case OP::ST: case OP::STI:
        ss << "a register and a label.";
        break;
    case OP::TRAP:
        return "TRAP instruction should not be used.\n";
    case OP::COUNT:
        return "Unexpected OP::COUNT value.\n";
    }
    ss << "\n";
    return ss.str();
}

std::string error_string(const POP::Type pop) {
    std::stringstream ss;
    ss << "Instruction " << pop << " must be followed by ";
    switch(pop) {
    case POP::ORIG:
        ss << "a valid address.";
        break;
    case POP::END:
        ss << "nothing else.";
        break;
    case POP::FILL:
        ss << "a valid word.";
        break;
    case POP::BLKW:
        ss << "a count and, optionally, a valid word to initialize every block with.";
        break;
    case POP::STRINGZ:
        ss << "a valid string.";
    case POP::COUNT:
        return "Unexpected POP::COUNT value.\n";
    }
    ss << "\n";
    return ss.str();
}

std::string error_string(const Token& tkn) {
    switch(tkn.getType()) {
    case TokenType::Instruction:
        return error_string(tkn.get<OP::Type>());
    case TokenType::PseudoOp:
        return error_string(tkn.get<POP::Type>());
    default:
        return "Unexpected TokenType.\n";
    }
}