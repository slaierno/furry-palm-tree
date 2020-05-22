#pragma once
#include <string>
#include <map>
#include <vector>
#include <array>
#include <variant>
#include <algorithm>
#include "lc3-hw.hpp"
#include "cx/cx.hpp"
#include "cx/cx_map.hpp"
#include "cx/cx_string.hpp"
#include "LabelMap.hpp"

/*********************************/
/*            MACROS             */
/*********************************/
#define TOKEN_TYPES \
    ENUM_MACRO(Instruction) \
    ENUM_MACRO(Label) \
    ENUM_MACRO(Register) \
    ENUM_MACRO(Number) \
    ENUM_MACRO(PseudoOp) \
    ENUM_MACRO(String) \
    ENUM_MACRO(Undefined)

#define OP_TYPES \
    /* 0*/ENUM_MACRO(RET) \
    /* 1*/ENUM_MACRO(RTI) \
    /* 2*/ENUM_MACRO(JSR) \
    /* 3*/ENUM_MACRO(BR) \
    /* 4*/ENUM_MACRO(BRn) \
    /* 5*/ENUM_MACRO(BRz) \
    /* 6*/ENUM_MACRO(BRp) \
    /* 7*/ENUM_MACRO(BRnz) \
    /* 8*/ENUM_MACRO(BRnp) \
    /* 9*/ENUM_MACRO(BRzp)  \
    /*10*/ENUM_MACRO(BRnzp) \
    /*11*/ENUM_MACRO(TRAP) \
    /*12*/ENUM_MACRO(JSRR) \
    /*13*/ENUM_MACRO(JMP) \
    /*14*/ENUM_MACRO(LD) \
    /*15*/ENUM_MACRO(ST) \
    /*16*/ENUM_MACRO(LDI) \
    /*17*/ENUM_MACRO(STI) \
    /*18*/ENUM_MACRO(LEA) \
    /*19*/ENUM_MACRO(NOT) \
    /*20*/ENUM_MACRO(ADD) \
    /*21*/ENUM_MACRO(AND) \
    /*22*/ENUM_MACRO(LDR) \
    /*23*/ENUM_MACRO(STR) \
    /*24*/ENUM_MACRO(LSHF) \
    /*25*/ENUM_MACRO(RSHFL) \
    /*26*/ENUM_MACRO(RSHFA) \
    /*27*/ENUM_MACRO(XOR) \
    /*28*/ENUM_MACRO(GETC) \
    /*29*/ENUM_MACRO(OUT) \
    /*30*/ENUM_MACRO(PUTS) \
    /*31*/ENUM_MACRO(IN) \
    /*32*/ENUM_MACRO(PUTSP) \
    /*33*/ENUM_MACRO(HALT)

#define POP_TYPES \
    ENUM_MACRO(ORIG) \
    ENUM_MACRO(FILL) \
    ENUM_MACRO(BLKW) \
    ENUM_MACRO(STRINGZ) \
    ENUM_MACRO(END)

#define REG_TYPES \
    ENUM_MACRO(R0) \
    ENUM_MACRO(R1) \
    ENUM_MACRO(R2) \
    ENUM_MACRO(R3) \
    ENUM_MACRO(R4) \
    ENUM_MACRO(R5) \
    ENUM_MACRO(R6) \
    ENUM_MACRO(R7)
    
/*********************************/
/*            ENUMS              */
/*********************************/

#define ENUM_MACRO(X) X,

/* Type of Tokens */
enum struct TokenType : unsigned {
    TOKEN_TYPES
    Count /* 8 */,
};

/* List of possible instructions */
namespace OP { 
    enum Type {
        OP_TYPES
        COUNT /* 34 */, 
    };
}

/* List of pseudo operations */
namespace POP {
    enum Type {
        POP_TYPES
        COUNT /* 5 */,
    };
}

/* List of addressable registers */
/* enum->int mapping shall be preserved */
namespace REG { 
    enum Type {
        REG_TYPES
        COUNT /* 8 */,
    };
}

#undef ENUM_MACRO

/* Value that can be assumed by a Token.
 * std::string variant holds for both strings and label.
 */
template<typename String>
using TokenValueGeneric = std::variant<OP::Type, 
                                       REG::Type, 
                                       POP::Type, 
                                       int, 
                                       String>;
using TokenValue = TokenValueGeneric<cx::string>;

/*********************************/
/*            MAPS               */
/*********************************/

static std::ostream& operator<<(std::ostream& os, const enum TokenType& type) {
    #define ENUM_MACRO(X) case TokenType::X: return os << "TokenType::" #X;
    switch(type) { TOKEN_TYPES; default: return os << "Invalid TokenType"; }
    #undef ENUM_MACRO
}
static std::ostream& operator<<(std::ostream& os, const enum OP::Type& type) {
    #define ENUM_MACRO(X) case OP::X: return os << "OP::" #X;
    switch(type) { OP_TYPES; default: return os << "Invalid OP::Type"; }
    #undef ENUM_MACRO
} 
static std::ostream& operator<<(std::ostream& os, const enum POP::Type& type) {
    #define ENUM_MACRO(X) case POP::X: return os << "POP::" #X;
    switch(type) { POP_TYPES; default: return os << "Invalid POP::Type"; }
    #undef ENUM_MACRO
} 
static std::ostream& operator<<(std::ostream& os, const enum REG::Type& type) {
    #define ENUM_MACRO(X) case REG::X: return os << "REG::" #X;
    switch(type) { REG_TYPES; default: return os << "Invalid REG::Type"; }
    #undef ENUM_MACRO
}
static std::ostream& operator<<(std::ostream& os, const TokenValue& v)
{
    std::visit([&os](auto&& arg) {os << arg;}, v);
    return os;
}

static inline auto case_insensitive_compare = [](std::string lhs, std::string rhs) {
    std::transform(lhs.begin(), lhs.end(), lhs.begin(), ::toupper);
    std::transform(rhs.begin(), rhs.end(), rhs.begin(), ::toupper);
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
};
static std::map<std::string, std::pair<TokenType, TokenValue>, decltype(case_insensitive_compare)> StringToTypeValuePairMap { {
#define ENUM_MACRO(X) {#X, {TokenType::Instruction, OP::X}},
    OP_TYPES
#undef ENUM_MACRO
#define ENUM_MACRO(X) {"."#X, {TokenType::PseudoOp, POP::X}},
    POP_TYPES
#undef ENUM_MACRO
#define ENUM_MACRO(X) {#X, {TokenType::Register, REG::X}},
    REG_TYPES
#undef ENUM_MACRO
}};

constexpr static char whitespace[] = " \t";
constexpr static char comment_sep[] = ";";

inline std::string trim_line(std::string line) {
    //return if line is empty
    if(size_t heading_pos = line.find_first_not_of(whitespace);
       heading_pos <= line.length()) {
        line = line.substr(heading_pos); //strip out heading whitespaces
        size_t comment_pos = line.find_first_of(comment_sep);
        line = line.substr(0, comment_pos); //strip out comments
        size_t trailing_pos = line.find_last_not_of(whitespace) + 1;
        line = line.substr(0, trailing_pos); //strip out trailing whitespaces
    }
    return line;
}

//Duplicate enums to map alias with HW opcode
enum {
    OP_RET = OP_JMP, 
    OP_BRn = OP_BR,
    OP_BRz = OP_BR,
    OP_BRp = OP_BR,
    OP_BRnz = OP_BR,
    OP_BRnp = OP_BR,
    OP_BRzp = OP_BR,
    OP_BRnzp = OP_BR,
    OP_JSRR = OP_JSR,
    OP_LSHF = OP_RES,
    OP_RSHFL = OP_RES,
    OP_RSHFA = OP_RES,
    OP_GETC = OP_TRAP,
    OP_OUT = OP_TRAP,
    OP_PUTS = OP_TRAP,
    OP_IN = OP_TRAP,
    OP_PUTSP = OP_TRAP,
    OP_HALT = OP_TRAP,
};

/* Array which associates every assembly instruction with its binary opcode */
/* Traps are not here, of course, since they should translate to TRAP instrunctions */
#define ENUM_MACRO(X) OP_ ## X,
constexpr std::array<uint16_t, OP::COUNT> opEnumToOpcodeMap {
    OP_TYPES
};
#undef ENUM_MACRO