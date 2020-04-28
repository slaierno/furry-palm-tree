#pragma once

#include <string>
#include "commons.hpp"

class Token {
    cx::string mToken;
    enum TokenType mType;
    TokenValue mValue;

public:
    /*********************************/
    /*          CTOR/DTOR            */
    /*********************************/
    constexpr Token(const enum TokenType type, const TokenValue& val) : mType(type), mValue(val) {}
    constexpr Token(const std::pair<const enum TokenType, const TokenValue>& pair) : Token(pair.first, pair.second) {}
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
    std::string getString() const { return std::string(get<cx::string>()); }

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

constexpr Token operator"" _tkn(const char* str, size_t size) {
    return Token(
        cx::map<cx::static_string, std::pair<enum TokenType, TokenValue>, OP::COUNT + POP::COUNT + REG::COUNT> {
        #define ENUM_MACRO(X) {#X, {TokenType::Instruction, OP::X}},
            OP_TYPES
        #undef ENUM_MACRO
        #define ENUM_MACRO(X) {"."#X, {TokenType::PseudoOp, POP::X}},
            POP_TYPES
        #undef ENUM_MACRO
        #define ENUM_MACRO(X) {#X, {TokenType::Register, REG::X}},
            REG_TYPES
        #undef ENUM_MACRO
        }[cx::static_string(str, size)]
    );
}