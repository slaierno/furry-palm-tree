#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <variant>
#include "commons.hpp"

using TokenValue = std::variant<enum OP, enum REG, int, std::string>;
struct Token {
    Token(std::string token);
    template<class T>
    Token(enum TokenType type, T value) : 
        mToken(""), mType(type), mValue(value) {
        static_assert(std::is_same<int        , T>::value ||
                      std::is_same<std::string, T>::value ||
                      std::is_same<const char*, T>::value ||
                      std::is_same<enum OP    , T>::value ||
                      std::is_same<enum REG   , T>::value);
    };
                        
    ~Token() {};

    enum TokenType mType;
    bool isNumber() const { 
        return mType == TokenType::Number || mType == TokenType::HexNumber; 
    }

    /*********************************/
    /*          GETTERs              */
    /*********************************/

    template<typename T> T get()                        const { return std::get<T>(mValue); }
    uint16_t               getNumValue(unsigned n = 16) const;
    uint16_t               getCondFlags()               const { return getCondFlags(get<enum OP>()); }; //TODO check if it is an OP
    static uint16_t getCondFlags(enum OP op) {
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
                //not a BR instruction
                return 0;
        }
    }
    
    /*********************************/
    /*         OPERATORS             */
    /*********************************/

    bool operator==(const Token& rhs) const;

    bool operator!=(const Token& rhs) const {
        return !(*this == rhs);
    }

    /*********************************/
    /*         TOSTRINGs             */
    /*********************************/

    std::string      getTokenString() const;
    std::string       getTypeString() const;
    std::string      getErrorString() const;

    friend void PrintTo(const Token& tkn, std::ostream* os) {
        *os << tkn.DebugString();  // whatever needed to print bar to os
    }

    std::string DebugString() const {
        std::stringstream ss;
        ss << "{ TokenType::" << getTypeString() << ", " << getTokenString() << " }";
        return ss.str();
    }
    
private:
    std::string mToken;
    TokenValue mValue;
};

using TokenList = std::vector<Token>;