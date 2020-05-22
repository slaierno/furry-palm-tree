#include "Token.hpp"
#include <cassert>
#include <regex>
#include <stdexcept>

/*********************************/
/*            CTOR               */
/*********************************/

Token::Token(const std::string& token) : mToken(token) {
    std::smatch label_match;
    const std::regex string_regex("^\"(.*)\"$");
    const std::regex num_regex("^[#x]([-+]?[0-9a-fA-F]+)$");
    // A label is valid if
    //  - It does not start with a number or an "r"/"R" followed by numbers only
    //  - It only contains alphanumeric characters and underscores
    const std::regex label_regex("^(?![rR]*[0-9]+$)[a-zA-Z0-9_]+$");
    if(auto search = StringToTypeValuePairMap.find(token); search != StringToTypeValuePairMap.end()) {
        // A keyword has been recognized
        mType = search->second.first;
        mValue = search->second.second;
    } else if (std::regex_match(token, label_match, string_regex)) {
        mType = TokenType::String;
        mValue.emplace<cx::string>(label_match[1].str());
    } else if (std::regex_match(token, label_match, num_regex)) {
        mType = TokenType::Number;
        const int base = (token.front() == '#') ? 10 : 16;
        //TODO catch invalid_argument exception
        mValue = std::stoi(label_match[1].str(), 0, base);
    } else if (std::regex_match(token, label_match, label_regex)) {
        mType = TokenType::Label;
        mValue.emplace<cx::string>(label_match[0].str());
    } else {
        mType = TokenType::Undefined;
    }
}

/*********************************/
/*          Utilities            */
/*********************************/

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
constexpr std::pair<int, int> RangeMap([[maybe_unused]] const T op) { return INVALID_RANGE; }

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

bool Token::checkRange(const Token& token) const {
    if(token.getType() != TokenType::Number) return false;
    auto [low, high] = std::visit([](auto&& arg) { return RangeMap(arg); }, mValue);
    return token.get<int>() >= low && token.get<int>() <= high;
}