#pragma once

#include <string>
#include <variant>
#include "commons.hpp"

struct Token {
    Token(std::string token);
    ~Token() {};

    enum TokenType mType;

    template<typename T> T get()            const { return std::get<T>(mValue); }
    std::string            getTokenString() const { return mToken; }
    uint16_t               getNumValue(unsigned int n = 16) const;
    bool                   isNumber()       const { return mType == TokenType::Number || mType == TokenType::HexNumber; }
    
    uint16_t getCondFlags() const {
        //TODO check if it is an OP
        switch(get<enum OP>()) {
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
                //not a BR instruction
                return 0;
        }
    }

private:
    std::string mToken;
    std::variant<enum OP, enum REG, int, std::string> mValue;
};

using TokenList = std::vector<Token>;