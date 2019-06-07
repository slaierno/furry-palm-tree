#pragma once
#include <iostream>
#include <sstream>
#include <exception>
#include "Token.hpp"
#include "commons.hpp"

namespace asm_error {
    // struct validation_exception : public std::logic_error {
    //     validation_exception(std::string const& msg) : std::logic_error(msg) { }
    // };

    struct label_not_found : public std::logic_error {
        label_not_found(std::string const& label) : std::logic_error(make_error_str(label)) {};
    private:
        static const std::string make_error_str(std::string const& label) {
            return "ERROR: label " + label + " not found!\n";
        }
    };

    struct invalid_format : public std::logic_error {
        invalid_format(TokenList const& tokens) : std::logic_error(make_error_str(tokens)) {};
    private:
        static const std::string make_error_str(TokenList const& tokens) {
            std::string argtype_str = "";
            for(auto const& token : tokens) {
                argtype_str += token.getErrorString() + " ";
            }
            argtype_str.pop_back(); //remove trailing space
            return "ERROR: Invalid instruction format " + argtype_str;
        }
    };

    struct invalid_trap_call : public std::logic_error {
        invalid_trap_call() : std::logic_error("ERROR: trap call can not be followed by anything else") {};
    };

    struct trap_inst_disabled : public std::logic_error {
        trap_inst_disabled() : std::logic_error("ERROR: TRAP instruction not allowed, use specific trap calls") {};
    };

    struct invalid_label_decl : public std::logic_error {
        invalid_label_decl() : std::logic_error("ERROR: label declarations can not be followed by anything else") {};
    };

    struct invalid_label_name : public std::logic_error {
        invalid_label_name(const Token& token) : std::logic_error(make_error_str(token)) {};
    private:
        static const std::string make_error_str(const Token& token) {
            return "ERROR: \"" + token.get<std::string>() + "\" is not a valid label name";
        }
    };

    struct duplicate_label : public std::logic_error {
        duplicate_label(const Token& token) : std::logic_error(make_error_str(token)) {};
    private:
        static const std::string make_error_str(const Token& token) {
            return "ERROR: label \"" + token.get<std::string>() + "\" has already been declared";
        }
    };

    struct invalid_token : public std::logic_error {
        invalid_token() : std::logic_error("ERROR: invalid token type") {};
    };

    struct unknown_pseudo_op : public std::logic_error {
        unknown_pseudo_op(const std::string& str) : std::logic_error("ERROR: unknown pseudo-op " + str) {};
    };

    struct invalid_instruction : public std::logic_error {
        invalid_instruction(const std::string& str) : std::logic_error(str) {};
    };

    struct invalid_string : public std::logic_error {
        invalid_string(const std::string& str) : std::logic_error(std::string("ERROR: invalid string format " + str + ". Maybe you forgot a quote?")) {};
    };

    struct invalid_pseudo_op : public std::logic_error {
        invalid_pseudo_op(std::string str = "invalid pseudo-op declaration") : std::logic_error("ERROR: " + str) {}
    };

    struct out_of_range_integer : public std::logic_error {
        out_of_range_integer(const int lower, const int upper) : std::logic_error(make_error_str(lower, upper)) {};
    private:
        static const std::string make_error_str(const int lower, const int upper) {
            std::stringstream sstream;
            sstream << "ERROR: int literal should have been between " << lower << " and " << upper;
            return sstream.str();
        }
    };
}