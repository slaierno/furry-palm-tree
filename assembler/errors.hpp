#pragma once
#include <iostream>
#include <sstream>
#include <exception>
#include "Token.hpp"
#include "commons.hpp"

namespace asm_error {

    struct generic_error : public std::logic_error {
        generic_error(const std::string& str) : std::logic_error(str) {};
    };

    // struct validation_exception : public generic_error {
    //     validation_exception(std::string const& msg) : generic_error(msg) { }
    // };

    struct label_not_found : public generic_error {
        label_not_found(std::string const& label) : generic_error(make_error_str(label)) {};
    private:
        static const std::string make_error_str(std::string const& label) {
            return "ERROR: label " + label + " not found!\n";
        }
    };

    struct invalid_format : public generic_error {
        invalid_format(TokenList const& tokens) : generic_error(make_error_str(tokens)) {};
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

    struct invalid_trap_call : public generic_error {
        invalid_trap_call() : generic_error("ERROR: trap call can not be followed by anything else") {};
    };

    struct trap_inst_disabled : public generic_error {
        trap_inst_disabled() : generic_error("ERROR: TRAP instruction not allowed, use specific trap calls") {};
    };

    struct invalid_label_decl : public generic_error {
        invalid_label_decl() : generic_error("ERROR: label declarations can not be followed by anything else") {};
    };

    struct invalid_label_name : public generic_error {
        invalid_label_name(const Token& token) : generic_error(make_error_str(token)) {};
    private:
        static const std::string make_error_str(const Token& token) {
            return "ERROR: \"" + token.get<std::string>() + "\" is not a valid label name";
        }
    };

    struct duplicate_label : public generic_error {
        duplicate_label(const Token& token) : generic_error(make_error_str(token)) {};
    private:
        static const std::string make_error_str(const Token& token) {
            return "ERROR: label \"" + token.get<std::string>() + "\" has already been declared";
        }
    };

    struct invalid_token : public generic_error {
        invalid_token() : generic_error("ERROR: invalid token type") {};
    };

    struct unknown_pseudo_op : public generic_error {
        unknown_pseudo_op(const std::string& str) : generic_error("ERROR: unknown pseudo-op " + str) {};
    };

    struct invalid_string : public generic_error {
        invalid_string(const std::string& str) : generic_error(std::string("ERROR: invalid string format " + str + ". Maybe you forgot a quote?")) {};
    };

    struct invalid_pseudo_op : public generic_error {
        invalid_pseudo_op(std::string str = "invalid pseudo-op declaration") : generic_error("ERROR: " + str) {}
    };

    struct out_of_range_integer : public generic_error {
        out_of_range_integer(const int lower, const int upper) : generic_error(make_error_str(lower, upper)) {};
    private:
        static const std::string make_error_str(const int lower, const int upper) {
            std::stringstream sstream;
            sstream << "ERROR: int literal should have been between " << lower << " and " << upper;
            return sstream.str();
        }
    };

    struct out_of_user_space : public generic_error {
        out_of_user_space(const int address) : generic_error(make_error_str(address)) {};
    private:
        static const std::string make_error_str(const int address) {
            std::stringstream sstream;
            sstream << "ERROR: you exceeded user space! " << std::hex << address;
            return sstream.str();
        }
    };

    struct unexpected : public generic_error {
        unexpected() : generic_error("FATAL: This is very bad, I don't even know what's going on. Debug the code or contact the developer.") {};
        unexpected(const std::string& str) : generic_error("FATAL: " + str) {};
    };

    struct todo : public generic_error {
        todo() : generic_error("TODO: This feature has not been implented yet.") {};
        todo(const std::string& str) : generic_error("TODO: " + str) {};
    };
}

namespace asm_warning {
    struct generic_warning : public std::logic_error {
        generic_warning(const std::string& str) : std::logic_error(str) {};
    };

    struct double_orig_decl : public generic_warning {
        double_orig_decl() : generic_warning(".orig was already declared, this one will be ignored") {};
    };

    struct inst_before_origin : public generic_warning {
        inst_before_origin() : generic_warning(".orig has not been declared, everything before will be ignored") {};
    };

    struct inst_after_end : public generic_warning {
        inst_after_end() : generic_warning(".end has already been declared, everything after will be ignored") {};
    };
}