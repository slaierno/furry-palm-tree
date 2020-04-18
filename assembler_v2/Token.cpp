#include "Token.hpp"
#include "/workspace/furry-palm-tree/assembler_v2/commons.hpp"
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
    if(auto search = StringToTypeValuePair.find(token); search != StringToTypeValuePair.end()) {
        // A keyword has been recognized
        mType = search->second.first;
        mValue = search->second.second;
    } else if (std::regex_match(token, label_match, string_regex)) {
        mType = TokenType::String;
        mValue = label_match[1].str();
    } else if (std::regex_match(token, label_match, num_regex)) {
        mType = TokenType::Number;
        const int base = (token.front() == '#') ? 10 : 16;
        //TODO catch invalid_argument exception
        mValue = std::stoi(label_match[1].str(), 0, base);
    } else if (std::regex_match(token, label_match, label_regex)) {
        mType = TokenType::Label;
        mValue = label_match[0].str();
    } else {
        mType = TokenType::Undefined;
    }
}

/*********************************/
/*          Utilities            */
/*********************************/

bool Token::checkRange(const Token& token) const {
    if(token.getType() != TokenType::Number) return false;
    auto [low, high] = RangeMap(mValue);
    return token.get<int>() >= low && token.get<int>() <= high;
}