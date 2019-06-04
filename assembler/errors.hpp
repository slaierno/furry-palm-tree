#include <iostream>
#include <sstream>
#include <exception>
#include "Token.hpp"
#include "commons.hpp"

namespace asm_error {
    // class validation_exception : public std::logic_error {
    // public:
    //     validation_exception(std::string const& msg) : std::logic_error(msg) { }
    // };

    class label_not_found : public std::logic_error {
    public:
        label_not_found(std::string const& label) : std::logic_error(make_error_str(label)) {}
    private:
        static const std::string make_error_str(std::string const& label) {
            return "ERROR: label " + label + " not found!\n";
        }
    };

    class invalid_format : public std::logic_error {
    public:
        invalid_format(TokenList const& tokens) : std::logic_error(make_error_str(tokens)) {};
    private:
        static const std::string make_error_str(TokenList const& tokens) {
            std::string argtype_str = "";
            for(auto const& token : tokens) {
                argtype_str += tokenToShortString(token) + " ";
            }
            argtype_str.pop_back(); //remove trailing space
            return "ERROR: Invalid instruction format " + argtype_str;
        }
    };

    class invalid_label_decl : public std::logic_error {
    public:
        invalid_label_decl() : std::logic_error("ERROR: label declarations can not be followed by anything else") {};
    };

    class invalid_label_name : public std::logic_error {
    public:
        invalid_label_name(const Token& token) : std::logic_error(make_error_str(token)) {};
    private:
        static const std::string make_error_str(const Token& token) {
            return "ERROR: \"" + token.get<std::string>() + "\" is not a valid label name";
        }
    };

    class duplicate_label : public std::logic_error {
    public:
        duplicate_label(const Token& token) : std::logic_error(make_error_str(token)) {};
    private:
        static const std::string make_error_str(const Token& token) {
            return "ERROR: label \"" + token.get<std::string>() + "\" has already been declared";
        }

    };

    class invalid_token : public std::logic_error {
    public:
        invalid_token() : std::logic_error("ERROR: unknown token type") {};
    };

    class out_of_range_integer : public std::logic_error {
    public:
        out_of_range_integer(const int nBits) : std::logic_error(make_error_str(nBits)) {};
    private:
        static const std::string make_error_str(const int nBits) {
            std::stringstream sstream;
            sstream << "ERROR: int literal should have been between -2^" << nBits-1 << " and 2^" << nBits-1 << "-1";
            return sstream.str();
        }
    };

    class out_of_range_integer_unsigned : public std::logic_error {
    public:
        out_of_range_integer_unsigned(const int nBits) : std::logic_error(make_error_str(nBits)) {};
    private:
        static const std::string make_error_str(const int nBits) {
            std::stringstream sstream;
            sstream << "ERROR: int literal should have been between 0 and 2^" << nBits << "-1";
            return sstream.str();
        }
    };
}