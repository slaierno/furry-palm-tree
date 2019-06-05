#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <variant>
#include "commons.hpp"

using TokenValue = std::variant<OP::Type, REG::Type, int, std::string>;
class Token {
    std::string mToken;
    TokenValue mValue;
    enum TokenType mType;
public:
                      Token(std::string token);
                     ~Token() {};
    template<class T> Token(enum TokenType type, T value) : 
        mToken(""), mType(type), mValue(value) {
        static_assert(std::is_same<int        , T>::value ||
                      std::is_same<std::string, T>::value ||
                      std::is_same<const char*, T>::value ||
                      std::is_same<OP::Type   , T>::value ||
                      std::is_same<REG::Type  , T>::value);
    };


    /*********************************/
    /*          GETTERs              */
    /*********************************/

    bool                   isNumber()                   const { return mType == TokenType::Number || mType == TokenType::HexNumber; }
    template<typename T> T get()                        const { return std::get<T>(mValue); }
    const enum TokenType   getType()                    const { return mType; }
     int16_t               getNumValue(unsigned n = 16) const;
    uint16_t               getCondFlags()               const;
    
    /*********************************/
    /*         OPERATORS             */
    /*********************************/

    bool operator==(const Token& rhs) const;
    bool operator!=(const Token& rhs) const { return !(*this == rhs); }

    /*********************************/
    /*         TOSTRINGs             */
    /*********************************/

    std::string      getTokenString() const;
    std::string       getTypeString() const;
    std::string      getErrorString() const;

    /*********************************/
    /*       GTEST HELPERS           */
    /*********************************/

    friend void PrintTo(const Token& tkn, std::ostream* os) {
        *os << tkn.DebugString();  // whatever needed to print bar to os
    }

    std::string DebugString() const {
        std::stringstream ss;
        ss << "{ TokenType::" << getTypeString() << ", " << getTokenString() << " }";
        return ss.str();
    }
};

using TokenList = std::vector<Token>;