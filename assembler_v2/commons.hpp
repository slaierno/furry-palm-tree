#pragma once
#include <string>
#include <map>
#include <vector>
#include <array>
#include <variant>
#include <algorithm>
#include "../lc3-hw.hpp"

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
    ENUM_MACRO(RET) \
    ENUM_MACRO(RTI) \
    ENUM_MACRO(JSR) \
    ENUM_MACRO(BR) \
    ENUM_MACRO(BRn) \
    ENUM_MACRO(BRz) \
    ENUM_MACRO(BRp) \
    ENUM_MACRO(BRnz) \
    ENUM_MACRO(BRnp) \
    ENUM_MACRO(BRzp)  \
    ENUM_MACRO(BRnzp) \
    ENUM_MACRO(TRAP) \
    ENUM_MACRO(JSRR) \
    ENUM_MACRO(JMP) \
    ENUM_MACRO(LD) \
    ENUM_MACRO(ST) \
    ENUM_MACRO(LDI) \
    ENUM_MACRO(STI) \
    ENUM_MACRO(LEA) \
    ENUM_MACRO(NOT) \
    ENUM_MACRO(ADD) \
    ENUM_MACRO(AND) \
    ENUM_MACRO(LDR) \
    ENUM_MACRO(STR) \
    ENUM_MACRO(LSHF) \
    ENUM_MACRO(RSHFL) \
    ENUM_MACRO(RSHFA) \
    ENUM_MACRO(XOR) \
    ENUM_MACRO(GETC) \
    ENUM_MACRO(OUT) \
    ENUM_MACRO(PUTS) \
    ENUM_MACRO(IN) \
    ENUM_MACRO(PUTSP) \
    ENUM_MACRO(HALT)

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
using TokenValue = std::variant<OP::Type, 
                                REG::Type, 
                                POP::Type, 
                                int, 
                                std::string>;

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

auto case_insensitive_compare = [](std::string lhs, std::string rhs) {
    std::transform(lhs.begin(), lhs.end(), lhs.begin(), ::toupper);
    std::transform(rhs.begin(), rhs.end(), rhs.begin(), ::toupper);
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
};
std::map<std::string, std::pair<TokenType, TokenValue>, decltype(case_insensitive_compare)> StringToTypeValuePair { {
#define ENUM_MACRO(X) {#X, {TokenType::Instruction, OP::X}},
    OP_TYPES
#undef ENUM_MACRO
#define ENUM_MACRO(X) {"."#X, {TokenType::PseudoOp, POP::X}},
    POP_TYPES
#undef ENUM_MACRO
#define ENUM_MACRO(X) {#X, {TokenType::Register, REG::X}},
    REG_TYPES
#undef ENUM_MACRO
}, case_insensitive_compare};

// TODO these can be vectors, they would be much more efficient but...
// Yes, they are NOT maps, but we are keeping names omogeneous!

/* Given N bits accepted by an [pseudo-]instruction, ranges are the
 * intervals [-2^(N-1),2^N-1]. 
 *   E.g., for 5 bits, we have [-16,31]. 
 * The compiler will trust the programmer, which should know that any 
 * number >2^(N-1)-1 will be sign extended.
 *   E.g. ADD R0 R1 #19
 *        #19 is going to be interpreted as 0b10011 and therefore
 *        the 5-bit sign-extension will make it -13 and the whole
 *        instruction will be R0:=R1-13
 * This is useful if the programmer wants to insert hex numbers,
 * which may be true with other instruction like .fill or .blkw
 *   E.g. MYLABEL .fill xFFF3
 *        MYLABEL will be filled with '1111 1111 1111 0011'.
 *        Writing:
 *          MYLABEL .fill x-D
 *          MYLABEL .fill #-13
 *        is going to be the same, but it may be counter-intuitive.
 *        Here the need to allow the extended range.
 * This choice is to make the compiler easier to write, but it
 * can be reverted.
 * 
 * EXCEPTIONS: 
 * - Shift operation will not accept negative values since they would make no sense.
 * - .orig will accept only positive values in the range [0x3000,0xFDFF]
 *
 * TODO consider accepting number in the range [2^(N-1), 2^N-1]
 *      only if they are in hex format.
 */

// This will always result in a false inclusion test
//   since ∀x,y∈ℤ,x>y ∄z∈ℤ|x<z<y
constexpr std::pair INVALID_RANGE(1,-1);

template<typename T>
constexpr std::pair<int, int> RangeMap(const T op) { return INVALID_RANGE; }

constexpr std::pair<int, int> RangeMap(const OP::Type op) {
    switch(op) {
    case OP::ADD:
    case OP::AND:
    case OP::XOR:
        //5 bits
        return std::pair(-16, 31);
    case OP::LDR:
    case OP::STR:
        //7 bits
        return std::pair(-32, 63);
    case OP::RSHFA:
    case OP::RSHFL:
    case OP::LSHF:
        return std::pair(0,15);
    default:
        return INVALID_RANGE;
    }
}

constexpr std::pair<int, int> RangeMap(const POP::Type pop) {
    switch(pop) {
    case POP::BLKW:
    case POP::FILL:
        return std::pair(-0x7FFF, 0xFFFF);
    case POP::ORIG:
        return std::pair(0x3000, 0xFDFF);
    default:
        return INVALID_RANGE;
    }
}

constexpr std::pair<int, int> RangeMap(const TokenValue& value) {
    return std::visit([](auto&& arg) { return RangeMap(arg); }, value);
}

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

/* A LabelMap is a map where [key,value] == [label name, address location] */
using LabelMap = std::map<std::string, uint16_t>;

#if 0
/* Map which ties instruction strings to their enum type */
#define o(N) {#N, OP::N}
const std::map<std::string, OP::Type> stringToOpEnumMap {
    o(RET)  , o(RTI)  , o(JSR)  , o(BR)   , o(BRn)  , o(BRz)  , o(BRp)  , o(BRnz) , o(BRnp) , o(BRzp) , 
    o(BRnzp), o(TRAP) , o(JSRR) , o(JMP)  , o(LD)   , o(ST)   , o(LDI)  , o(STI)  , o(LEA)  , o(NOT)  , 
    o(ADD)  , o(AND)  , o(LDR)  , o(STR)  , o(LSHF) , o(RSHFL), o(RSHFA), o(XOR)  , o(GETC) , o(OUT)  , 
    o(PUTS) , o(IN)   , o(PUTSP), o(HALT)
};
#undef o

/* Array which associates every assembly instruction with its binary opcode */
/* Traps are not here, of course, since the should translate to TRAP instrunctions */
#define o(N) OP_ ## N
const std::array<uint16_t, OP::COUNT> opEnumToOpcodeMap {
    o(JMP), o(RTI), o(JSR) , o(BR) , o(BR) , o(BR) , o(BR) , o(BR) , o(BR),
    o(BR) , o(BR) , o(TRAP), o(JSR), o(JMP), o(LD) , o(ST) , o(LDI), o(STI), 
    o(LEA), o(NOT), o(ADD) , o(AND), o(LDR), o(STR), o(RES), o(RES), o(RES),
    o(XOR)
};
#undef o

/* Map which ties pseudo-op strings to their enum type */
#define p(N) {#N, POP::N}
const std::map<std::string, POP::Type> stringToPOpEnumMap {
    p(ORIG), p(FILL), p(BLKW), p(STRINGZ), p(END)
};
#undef p

#define t(N) TRAP_ ## N
const std::array<uint16_t, 7> trapEnumToOpcodeMap {
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
const std::vector<enum TokenType> Num {TokenType::Number};
const std::vector<enum TokenType> Reg_Label {TokenType::Register, TokenType::Label};
const std::vector<enum TokenType> Reg_Reg {TokenType::Register, TokenType::Register};
const std::vector<enum TokenType> Reg_Reg_Reg {TokenType::Register, TokenType::Register, TokenType::Register};
const std::vector<enum TokenType> Reg_Reg_Num {TokenType::Register, TokenType::Register, TokenType::Number};

/* Map which associates every instruction with every possible arguments combination */
const std::multimap<OP::Type, const std::vector<enum TokenType>> validInstructionMap {
    {OP::RET, NO_ARGS}      , {OP::RTI, NO_ARGS}      , {OP::JSR, Label},
    {OP::BR, Label}         , {OP::BRn, Label}        , {OP::BRz, Label},
    {OP::BRp, Label}        , {OP::BRnz, Label}       , {OP::BRnp, Label},
    {OP::BRzp, Label}       , {OP::BRnzp, Label}      , {OP::TRAP, Num},
    {OP::JSRR, Reg}         , {OP::JMP, Reg}          , {OP::LD, Reg_Label},
    {OP::ST, Reg_Label}     , {OP::LDI, Reg_Label}    , {OP::STI, Reg_Label},
    {OP::LEA, Reg_Label}    , {OP::NOT, Reg_Reg}      , {OP::ADD, Reg_Reg_Reg},
    {OP::ADD, Reg_Reg_Num}  , {OP::AND, Reg_Reg_Reg}  , {OP::AND, Reg_Reg_Num},
    {OP::LDR, Reg_Reg_Num}  , {OP::STR, Reg_Reg_Num}  , {OP::LSHF, Reg_Reg_Num}, 
    {OP::RSHFL, Reg_Reg_Num}, {OP::RSHFA, Reg_Reg_Num}, {OP::XOR, Reg_Reg_Reg},
    {OP::XOR, Reg_Reg_Num}
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
#endif