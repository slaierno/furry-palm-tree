#pragma once
#include <string>
#include <sstream>
#include <iomanip>

std::string getOffset(size_t nbit, uint16_t instr) {
    uint16_t mask = (1 << nbit) - 1;
    size_t hex_digit = (nbit + 3) / 4;
    std::stringstream ss;
    ss << std::uppercase << std::setfill('0') << std::setw(hex_digit) << std::hex << (instr & mask);
    return ss.str();
}

std::string mcodeToString(uint16_t instr) {
    std::stringstream ss;
    uint16_t op = instr >> 12;
    switch(op) {        
    case OP_BR:    /* branch */
    {
        ss << "BR";
        if(instr >> 11 & 0x1) ss << "n";
        if(instr >> 10 & 0x1) ss << "z";
        if(instr >>  9 & 0x1) ss << "p";
        ss << " #" << getOffset(9, instr);
    }
    break;
    case OP_ADD:    /* add  */
    case OP_AND:    /* bitwise and */
    case OP_XOR:    /* bitwise xor */
    {
        switch(op) {
        case OP_ADD: ss << "ADD "; break;
        case OP_AND: ss << "AND "; break;
        case OP_XOR: ss << "XOR "; break;
        }
        uint8_t dr = (instr >> 9) & 0x7;
        uint8_t sr1 = (instr >> 6) & 0x7;
        ss << "R" << (int)dr << " R" << (int)sr1 << " ";
        if((instr >> 5) & 0x1) {
            int8_t imm5 = instr & 0x1F;
            if(imm5 & 0x10) imm5 |= 0xE0;
            ss << "#" << (int)imm5;
        } else {
            uint8_t sr2 = instr & 0x7;
            ss << "R" << (int)sr2;
        }
    }
    break;
    case OP_SHF:    /* reserved (bit shift) */
    {
        bool right = instr & 0x10;
        bool aritm = instr & 0x20;
        ss << (right?"R":"L") << "SHF";
        if(right) ss << (aritm?"A":"L");
        ss << " ";
        uint8_t dr = (instr >> 9) & 0x7;
        uint8_t sr1 = (instr >> 6) & 0x7;
        uint8_t imm4 = instr & 0xF;
        ss << "R" << (int)dr << " R" << (int)sr1 << " #" << (int)imm4;
    }
    break;
    case OP_LD :    /* load */
    case OP_LDI:    /* load indirect */
    case OP_LEA:    /* load effective address */
    case OP_ST :    /* store */
    case OP_STI:    /* store indirect */
    {
        switch(op) {
        case OP_LD : ss << "LD "; break;
        case OP_LDI: ss << "LDI "; break;
        case OP_LEA: ss << "LEA "; break;
        case OP_ST : ss << "ST "; break;
        case OP_STI: ss << "STI "; break;
        }
        uint8_t dr = (instr >> 9) & 0x7;
        ss << "R" << (int)dr << " #" << getOffset(9, instr);
    }
    break;
    case OP_JSR:    /* jump register */
    {
        if((instr >> 11) & 0x1) { //JSR
            ss << "JSR #" << getOffset(11, instr);
        } else { //JSRR
            uint8_t br = (instr >> 6) & 0x7;
            ss << "JSR R" << (int)br;
        }
    }
    break;
    case OP_LDR:    /* load register */
    case OP_STR:    /* store register */
    {
        switch(op) {
        case OP_LDR: ss << "LDR "; break;
        case OP_STR: ss << "STR "; break;
        }
        uint8_t dr = (instr >> 9) & 0x7;
        uint8_t br = (instr >> 6) & 0x7;
        ss << "R" << (int)dr << " R" << (int)br << " #" << getOffset(6, instr);
    }
    break;
    case OP_RTI:    /* unused */
        ss << "RTI";
    break;
    case OP_JMP:    /* jump */
    {
        uint8_t br = (instr >> 6) & 0x7;
        ss << "JMP R" << (int)br;
    }
    break;
    case OP_TRAP:    /* execute trap */
    {
        switch(instr & 0xFF) {
        case TRAP_GETC:  ss << "GETC"; break;
        case TRAP_OUT:   ss << "OUT"; break;
        case TRAP_PUTS:  ss << "PUTS"; break;
        case TRAP_IN:    ss << "IN"; break;
        case TRAP_PUTSP: ss << "PUTSP"; break;
        case TRAP_HALT:  ss << "HALT"; break;
        }
    }
    break;
    }
    return ss.str();
}