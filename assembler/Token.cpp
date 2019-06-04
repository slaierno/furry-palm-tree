#include <type_traits>
#include "Token.hpp"
#include "utils.hpp"
#include "errors.hpp"

Token::Token(std::string token) : mToken(token) {
    if(op_tokens.find(token) != op_tokens.end()) {
        mType = TokenType::Instruction;
        mValue = op_tokens.find(token)->second;
    } 
    else if(reg_tokens.find(token) != reg_tokens.end()) {
        mType = TokenType::Register;
        mValue = reg_tokens.find(token)->second;
    }
    else if(token[0] == '#') {
        try {
            if(token[1] == 'x') {
                mType = TokenType::HexNumber;
                mValue = std::stoul(token.substr(2), 0, 16);
            } else {
                mType = TokenType::Number;
                mValue = std::stol(token.substr(1), 0, 10);
            }
        } catch (std::invalid_argument& e) {
            //TODO error
        }
    } else {
        mType = TokenType::Label;
        mValue = token;
    }
}

std::string Token::getTokenString() const {
    switch(mType) {
    case TokenType::Instruction:
        return op_token_to_string(get<enum OP>());
    case TokenType::Register:
        return reg_token_to_string(get<enum REG>());
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

uint16_t Token::getNumValue(unsigned int n) const {
    switch (mType) {
    case TokenType::Instruction:
        return op_map.find(get<enum OP>())->second;
    case TokenType::Register:
        return reg_map.find(get<enum REG>())->second;
    case TokenType::Number:
    case TokenType::HexNumber: {
        int value = get<int>();
        uint16_t mask = 0xFFFF >> (16 - n);
        return (uint16_t)(value & mask);
    }
    case TokenType::Label: {
        const std::string label_str = get<std::string>();
        auto it = label_map.find(label_str);
        if(it == label_map.end()) {
            throw asm_error::label_not_found(label_str);
        }
        uint16_t label_address = it->second;
        uint16_t mask = 0xFFFF >> (16 - n);
        return (label_address - inst_address) & mask;
    }
    default:
        //TODO error
        return 0;
    }
    return 0;
}

bool Token::operator==(const Token& rhs) const{
    const auto& lhs = *this;
    if(lhs.mType == rhs.mType) {
        switch(lhs.mType) {
            case TokenType::Instruction:
                return lhs.get<enum OP>() == rhs.get<enum OP>();
            case TokenType::Register:
                return lhs.get<enum REG>() == rhs.get<enum REG>();
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