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
    constexpr T get() const {
        // if constexpr(std::is_same_v<T, std::string>) {
        //     return std::string(std::get<std::string_view>(mValue));
        // } else 
        if constexpr(cx::holds_variant_v<T, TokenValue>) {
            //TODO throw something for type mismatch
            return std::get<T>(mValue);
        } else if constexpr(std::is_integral_v<T>) {
            if (const auto& val = get<int>(); val >= std::numeric_limits<T>::min() && val <= std::numeric_limits<T>::max())
                return static_cast<T>(val);
            else 
                throw std::logic_error("Out of range!\n"); //TODO throw meaningful error
        } else {
            static_assert(cx::fail_v<T>, "Invalid get<> template type.");
        }
    }

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