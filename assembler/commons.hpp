#pragma once
#include <map>
#include <string>
#include <vector>
#include "../lc3-hw.hpp"

/* Type of Tokens
 * TODO consider merging Number and HexNumber
 * TODO consider adding Assembly elements (addresses, variables, etc)
 */
enum struct TokenType {
    Instruction,
    Label,
    Register,
    Number,
    HexNumber,
    Undefined
};

/*********************************/
/*          OPCODES              */
/*********************************/

/* List of possible instructions */
enum OP {
    RET = 0, RTI, 
    JSR, BR, BRn, BRz, BRp, BRnz, BRnp, BRzp, BRnzp,
    TRAP,
    JSRR, JMP,
    LD, ST, LDI, STI, LEA,
    NOT,
    ADD, AND,
    LDR, STR, LSHF, RSHFL, RSHFA,
    COUNT
};

/* Map which ties instruction strings to their enum type */
#define o(N) {#N, OP::N}
const std::map<std::string, enum OP> op_tokens {
    o(RET), o(RTI),
    o(JSR), o(BR), o(BRn), o(BRz), o(BRp), o(BRnz), o(BRnp), o(BRzp), o(BRnzp),
    o(TRAP),
    o(JSRR), o(JMP),
    o(LD), o(ST), o(LDI), o(STI), o(LEA),
    o(NOT),
    o(ADD), o(AND),
    o(LDR), o(STR), o(LSHF), o(RSHFL), o(RSHFA),
};
#undef o

/* This is slow, used only for debug strings */
static std::string op_token_to_string(enum OP op) {
    for(auto const& el : op_tokens) {
        if(el.second == op) return el.first;
    }
    return "INVALID";
};

/* Map which associates every assembly instruction with its binary opcode */
#define op(N) {OP::N, OP_ ## N}
#define alias(M,N) {OP::M, OP_ ## N}
#define br(c) {OP::BR ## c, OP_BR}
const std::map<enum OP, uint16_t> op_map {
    op(RTI),
    op(JSR), alias(JSRR, JSR),
    br(), br(n), br(z), br(p), br(nz), br(np), br(zp), br(nzp),
    op(TRAP),
    op(JMP), alias(RET, JMP),
    op(LD),
    op(ST),
    op(LDI),
    op(STI),
    op(LEA),
    op(NOT),
    op(ADD),
    op(AND),
    op(LDR),
    op(STR),
    alias(LSHF, RES), alias(RSHFL, RES), alias(RSHFA, RES),
};
#undef op
#undef alias
#undef br

/*********************************/
/*          REGISTERS            */
/*********************************/

/* List of addressable registers */
enum struct REG {
    R0, R1, R2, R3, R4, R5, R6, R7,
    COUNT
};

/* Map which ties register strings to their enum type */
#define o(N) {#N, REG::N}
const std::map<std::string, enum REG> reg_tokens {
    o(R0), o(R1), o(R2), o(R3), o(R4), o(R5), o(R6), o(R7),
};
#undef o

/* This is slow, used only for debug strings */
static std::string reg_token_to_string(enum REG reg) {
    for(auto const& el : reg_tokens) {
        if(el.second == reg) return el.first;
    }
    return "INVALID";
};

#define reg(N) {REG::N, R_ ## N}
const std::map<enum REG, uint16_t> reg_map {
    reg(R0),
    reg(R1),
    reg(R2),
    reg(R3),
    reg(R4),
    reg(R5),
    reg(R6),
    reg(R7),
};
#undef reg

/*********************************/
/*         INSTRUCTIONS          */
/*********************************/

/* These consts represents all the possible arguments combinations */
const std::vector<enum TokenType> NO_ARGS {};
const std::vector<enum TokenType> LABEL {TokenType::Label};
const std::vector<enum TokenType> REG {TokenType::Register};
const std::vector<enum TokenType> HEX {TokenType::HexNumber};
const std::vector<enum TokenType> REG_LABEL {TokenType::Register, TokenType::Label};
const std::vector<enum TokenType> REG_REG {TokenType::Register, TokenType::Register};
const std::vector<enum TokenType> REG_REG_REG {TokenType::Register, TokenType::Register, TokenType::Register};
const std::vector<enum TokenType> REG_REG_HEX {TokenType::Register, TokenType::Register, TokenType::HexNumber};
const std::vector<enum TokenType> REG_REG_NUM {TokenType::Register, TokenType::Register, TokenType::Number};

/* Map which associates every instruction with every possible arguments combination */
const std::multimap<OP, const std::vector<enum TokenType>> validation_map {
    {OP::RET, NO_ARGS},
    {OP::RTI, NO_ARGS},
    {OP::JSR, LABEL},
    {OP::BR, LABEL},
    {OP::BRn, LABEL},
    {OP::BRz, LABEL},
    {OP::BRp, LABEL},
    {OP::BRnz, LABEL},
    {OP::BRnp, LABEL},
    {OP::BRzp, LABEL},
    {OP::BRnzp, LABEL},
    {OP::TRAP, HEX},
    {OP::JSRR, REG},
    {OP::JMP, REG},
    {OP::LD, REG_LABEL},
    {OP::ST, REG_LABEL},
    {OP::LDI, REG_LABEL},
    {OP::STI, REG_LABEL},
    {OP::LEA, REG_LABEL},
    {OP::NOT, REG_REG},
    {OP::ADD, REG_REG_REG},
    {OP::ADD, REG_REG_NUM},
    {OP::ADD, REG_REG_HEX},
    {OP::AND, REG_REG_REG},
    {OP::AND, REG_REG_NUM},
    {OP::AND, REG_REG_HEX},
    {OP::LDR, REG_REG_NUM},
    {OP::LDR, REG_REG_HEX},
    {OP::STR, REG_REG_NUM},
    {OP::STR, REG_REG_HEX},
    {OP::LSHF, REG_REG_NUM},
    {OP::LSHF, REG_REG_HEX},
    {OP::RSHFL, REG_REG_NUM},
    {OP::RSHFL, REG_REG_HEX},
    {OP::RSHFA, REG_REG_NUM},
    {OP::RSHFA, REG_REG_HEX},
};