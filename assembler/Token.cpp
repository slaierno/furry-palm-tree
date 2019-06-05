#include <type_traits>
#include "Token.hpp"
#include "utils.hpp"
#include "errors.hpp"

/*********************************/
/*            CTOR               */
/*********************************/

Token::Token(std::string token) : mToken(token) {
    if(stringToOpEnumMap.find(token) != stringToOpEnumMap.end()) {
        mType = TokenType::Instruction;
        mValue = stringToOpEnumMap.find(token)->second;
    } 
    else if(stringToRegEnumMap.find(token) != stringToRegEnumMap.end()) {
        mType = TokenType::Register;
        mValue = stringToRegEnumMap.find(token)->second;
    }
    else if(token[0] == '#') {
        if(token[1] == 'x') {
            mType = TokenType::HexNumber;
            mValue = std::stoul(token.substr(2), 0, 16);
        } else {
            mType = TokenType::Number;
            mValue = std::stol(token.substr(1), 0, 10);
        }
    } else {
        mType = TokenType::Label;
        mValue = token;
    }
}

/*********************************/
/*          GETTERs              */
/*********************************/

int16_t Token::getNumValue(unsigned int n) const {
    switch (mType) {
    case TokenType::Instruction:
        return opEnumToOpcodeMap[get<OP::Type>()];
    case TokenType::Register:
        return regEnumToOpcodeMap[get<REG::Type>()];
    case TokenType::Number:
    case TokenType::HexNumber: {
        int value = get<int>();
        uint16_t mask = 0xFFFF >> (16 - n);
        return (int16_t)(value & mask);
    }
    case TokenType::Label: {
        const std::string label_str = get<std::string>();
        auto it = label_map.find(label_str);
        if(it == label_map.end()) {
            throw asm_error::label_not_found(label_str);
        }
        uint16_t label_address = it->second;
        uint16_t mask = 0xFFFF >> (16 - n);
        return (int16_t)((label_address - inst_address) & mask);
    }
    default:
        //TODO error
        return 0;
    }
    return 0;
}

uint16_t Token::getCondFlags() const {
    uint8_t cf = brToCondFlag(get<OP::Type>()); 
    if(cf) return cf; 
    else throw asm_error::invalid_instruction("trying to get condition flags from a non-BR instruction");
};

/*********************************/
/*         TOSTRINGs             */
/*********************************/

std::string Token::getTokenString() const {
    switch(mType) {
    case TokenType::Instruction:
        return enumToString(get<OP::Type>());
    case TokenType::Register:
        return enumToString(get<REG::Type>());
    case TokenType::Number:
    case TokenType::HexNumber:
        return std::to_string(get<int>());
    case TokenType::Label:
        return get<std::string>();
    default:
        throw std::logic_error("Unknown TokenType");
    }
}

std::string Token::getTypeString() const {
    switch(mType) {
    case TokenType::Instruction:
        return "Instruction";
    case TokenType::Label:
        return "Label";
    case TokenType::Register:
        return "Register";
    case TokenType::Number:
        return "Number";
    case TokenType::HexNumber:
        return "HexNumber";
    case TokenType::Undefined:
        return "Undefined";
    default:
        throw std::logic_error("Unknown TokenType");
    }
}

std::string Token::getErrorString() const {
    switch(mType) {
    case TokenType::Instruction:
        return getTokenString();
    case TokenType::Label:
        return "LABEL";
    case TokenType::Register:
        return "REG";
    case TokenType::Number:
    case TokenType::HexNumber:
        return "NUM";
    case TokenType::Undefined:
        return "UNDEF";
    default:
        throw std::logic_error("Unknown TokenType");
    }
}

/*********************************/
/*         OPERATORS             */
/*********************************/

bool Token::operator==(const Token& rhs) const{
    const auto& lhs = *this;
    if(lhs.mType == rhs.mType) {
        switch(lhs.mType) {
            case TokenType::Instruction:
                return lhs.get<OP::Type>() == rhs.get<OP::Type>();
            case TokenType::Register:
                return lhs.get<REG::Type>() == rhs.get<REG::Type>();
            case TokenType::Number:
            case TokenType::HexNumber:
                return lhs.get<int>() == rhs.get<int>();
            case TokenType::Label:
                return lhs.get<std::string>() == rhs.get<std::string>();
            default:
                return false;
        }
    }
    return false;
}