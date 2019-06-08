#pragma once
#include <string>
#include <map>
#include <vector>
#include <array>
#include "../lc3-hw.hpp"

/*********************************/
/*            ENUMS              */
/*********************************/

/* Type of Tokens
 * TODO consider adding Assembly elements (addresses, variables, etc)
 */
enum struct TokenType {
    Instruction,
    Label,
    Register,
    Number,
    HexNumber,
    PseudoOp,
    Trap,
    String,
    Undefined
};

/* List of possible instructions */
namespace OP { 
    enum Type {
        RET , RTI  , JSR , BR  , BRn, BRz , BRp , BRnz , BRnp ,
        BRzp, BRnzp, TRAP, JSRR, JMP, LD  , ST  , LDI  , STI  , 
        LEA , NOT  , ADD , AND , LDR, STR , LSHF, RSHFL, RSHFA,
        COUNT //27
    };
}

/* List of pseudo operations */
namespace POP {
    enum Type {
        ORIG, FILL, BLKW, STRINGZ, END,
        COUNT //5
    };
}

namespace TRAP {
    enum Type {
        GETC, OUT, PUTS, IN, PUTSP, HALT,
        COUNT //6
    };
}

/* List of addressable registers */
namespace REG { 
    enum Type {
        R0, R1, R2, R3, R4, R5, R6, R7,
        COUNT //8
    };
}


/*********************************/
/*            MAPS               */
/*********************************/

/* Map which ties instruction strings to their enum type */
#define o(N) {#N, OP::N}
const std::map<std::string, OP::Type> stringToOpEnumMap {
    o(RET) , o(RTI)  , o(JSR) , o(BR)  , o(BRn), o(BRz), o(BRp) , o(BRnz) , o(BRnp) , 
    o(BRzp), o(BRnzp), o(TRAP), o(JSRR), o(JMP), o(LD) , o(ST)  , o(LDI)  , o(STI)  , 
    o(LEA) , o(NOT)  , o(ADD) , o(AND) , o(LDR), o(STR), o(LSHF), o(RSHFL), o(RSHFA),
};
#undef o

/* Map which associates every assembly instruction with its binary opcode */
#define o(N) OP_ ## N
const std::array<uint16_t, OP::COUNT> opEnumToOpcodeMap {
    o(JMP), o(RTI), o(JSR) , o(BR) , o(BR) , o(BR) , o(BR) , o(BR) , o(BR),
    o(BR) , o(BR) , o(TRAP), o(JSR), o(JMP), o(LD) , o(ST) , o(LDI), o(STI), 
    o(LEA), o(NOT), o(ADD) , o(AND), o(LDR), o(STR), o(RES), o(RES), o(RES),
};
#undef o

/* Map which ties pseudo-op strings to their enum type */
#define p(N) {#N, POP::N}
const std::map<std::string, POP::Type> stringToPOpEnumMap {
    p(ORIG), p(FILL), p(BLKW), p(STRINGZ), p(END)
};
#undef p

/* Map which ties trap strings to their enum type */
#define t(N) {#N, TRAP::N}
const std::map<std::string, TRAP::Type> stringToTrapEnumMap {
    t(GETC), t(OUT), t(PUTS), t(IN), t(PUTSP), t(HALT)
};
#undef t

#define t(N) TRAP_ ## N
const std::array<uint16_t, TRAP::COUNT> trapEnumToOpcodeMap {
    t(GETC), t(OUT), t(PUTS), t(IN), t(PUTSP), t(HALT)
};
#undef t

/* Map which ties register strings to their enum type */
#define r(N) {#N, REG::N}
const std::map<std::string, REG::Type> stringToRegEnumMap {
    r(R0), r(R1), r(R2), r(R3), r(R4), r(R5), r(R6), r(R7),
};
#undef r

#define r(N) R_ ## N
const std::array<uint16_t, REG::COUNT> regEnumToOpcodeMap {
    r(R0), r(R1), r(R2), r(R3), r(R4), r(R5), r(R6), r(R7),
};
#undef r

/* These consts represents all the possible arguments combinations for OP type*/
const std::vector<enum TokenType> NO_ARGS {};
const std::vector<enum TokenType> Label {TokenType::Label};
const std::vector<enum TokenType> Reg {TokenType::Register};
const std::vector<enum TokenType> Hex {TokenType::HexNumber};
const std::vector<enum TokenType> Reg_Label {TokenType::Register, TokenType::Label};
const std::vector<enum TokenType> Reg_Reg {TokenType::Register, TokenType::Register};
const std::vector<enum TokenType> Reg_Reg_Reg {TokenType::Register, TokenType::Register, TokenType::Register};
const std::vector<enum TokenType> Reg_Reg_Hex {TokenType::Register, TokenType::Register, TokenType::HexNumber};
const std::vector<enum TokenType> Reg_Reg_Num {TokenType::Register, TokenType::Register, TokenType::Number};

/* Map which associates every instruction with every possible arguments combination */
const std::multimap<OP::Type, const std::vector<enum TokenType>> validInstructionMap {
    {OP::RET, NO_ARGS}      , {OP::RTI, NO_ARGS}      , {OP::JSR, Label},
    {OP::BR, Label}         , {OP::BRn, Label}        , {OP::BRz, Label},
    {OP::BRp, Label}        , {OP::BRnz, Label}       , {OP::BRnp, Label},
    {OP::BRzp, Label}       , {OP::BRnzp, Label}      , {OP::TRAP, Hex},
    {OP::JSRR, Reg}         , {OP::JMP, Reg}          , {OP::LD, Reg_Label},
    {OP::ST, Reg_Label}     , {OP::LDI, Reg_Label}    , {OP::STI, Reg_Label},
    {OP::LEA, Reg_Label}    , {OP::NOT, Reg_Reg}      , {OP::ADD, Reg_Reg_Reg},
    {OP::ADD, Reg_Reg_Num}  , {OP::ADD, Reg_Reg_Hex}  , {OP::AND, Reg_Reg_Reg},
    {OP::AND, Reg_Reg_Num}  , {OP::AND, Reg_Reg_Hex}  , {OP::LDR, Reg_Reg_Num},
    {OP::LDR, Reg_Reg_Hex}  , {OP::STR, Reg_Reg_Num}  , {OP::STR, Reg_Reg_Hex},
    {OP::LSHF, Reg_Reg_Num} , {OP::LSHF, Reg_Reg_Hex} , {OP::RSHFL, Reg_Reg_Num},
    {OP::RSHFL, Reg_Reg_Hex}, {OP::RSHFA, Reg_Reg_Num}, {OP::RSHFA, Reg_Reg_Hex},
};

/*********************************/
/*          FUNCTIONS            */
/*********************************/

/* These are slow, used only for debug strings */
template<typename T> static std::string enumToString(T en);
template<>           inline std::string enumToString(const OP::Type  en) {
    for(auto&& [key,val] : stringToOpEnumMap) if(en == val) return key;
    return "INVALID";
};
template<>           inline std::string enumToString(const REG::Type en) {
    for(auto&& [key,val] : stringToRegEnumMap) if(en == val) return key;
    return "INVALID";
};

static inline uint8_t brToCondFlag(const OP::Type op) {
    switch(op) {
    case OP::BR:
    case OP::BRnzp:
        return 0b111;
    case OP::BRn:
        return 0b100;
    case OP::BRz:
        return 0b010;
    case OP::BRp:
        return 0b001;
    case OP::BRnz:
        return 0b110;
    case OP::BRnp:
        return 0b101;
    case OP::BRzp:
        return 0b011;
    default:
        return 0;
    }
}