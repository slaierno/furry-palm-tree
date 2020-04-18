#pragma once

#include <string>
#include "commons.hpp"

class Token {
    std::string mToken;
    enum TokenType mType;
    TokenValue mValue;

public:
    /*********************************/
    /*          CTOR/DTOR            */
    /*********************************/
    Token(const enum TokenType type, TokenValue val) : mType(type), mValue(val) {}
              Token(const std::string& token);

    /*********************************/
    /*          Utilities            */
    /*********************************/
    bool checkRange(const Token& token) const;

    /*********************************/
    /*          GETTERs              */
    /*********************************/
    constexpr enum TokenType getType() const { return mType; }
    //TODO should give an error if mType is Undefined or does not match with T
    template<typename T>
    constexpr T get() const { return std::get<T>(mValue); }

    /*********************************/
    /*         OPERATORS             */
    /*********************************/
    constexpr bool operator==(const Token& rhs) const { return this->mType == rhs.mType && this->mValue == rhs.mValue; }
    constexpr bool operator!=(const Token& rhs) const { return !(*this == rhs); }

    /*********************************/
    /*         OPERATOR <<           */
    /*********************************/
    friend std::ostream& operator<<(std::ostream& os, const Token& tkn) {
        return os << "{ " << tkn.mType << ", " << tkn.mValue << " }"; 
    }
};

namespace TokenConsts{
#define ENUM_MACRO(X) const Token X = Token(TokenType::Instruction, OP::X);
OP_TYPES
#undef ENUM_MACRO
#define ENUM_MACRO(X) const Token X = Token(TokenType::PseudoOp, POP::X);
POP_TYPES
#undef ENUM_MACRO
}