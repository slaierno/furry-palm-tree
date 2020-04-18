#include <iostream>
#include <fstream>
#include <regex>
#include <deque>
#include <stdexcept>
#include "errors.hpp"
#include "commons.hpp"
#include "assembler.hpp"

/*********************************/
/*          UTILITIES            */
/*********************************/

uint16_t address_increment(const Instruction& inst) {
    if (inst.front().getType() == TokenType::PseudoOp) {
        switch(inst.front().get<POP::Type>()) {
        case POP::STRINGZ:
            return inst.back().get<std::string>().length() + 1; //remember the NUL character
        case POP::BLKW:
            return inst[1].get<int>();
        case POP::ORIG: case POP::END:
            return 0;
        default:
            return 1;
        }
    }
    return 1;
}

/*********************************/
/*       ASSEMBLER STEPS         */
/*********************************/

/* STEP 1 = TOKENIZER
 * Separates assembly files into a deque of tokens.
 *
 * This step checks for single-token validity, but does not check for any
 * syntax or semantic error.
 * E.g.
 *    ADD AND #123 PIPPO    => Pass this step, every single Token is valid
 *    ADD R0 R1 R8          => Does not pass this step, R8 is not a valid Token
 *
 * TODO there should be a special token for hex number without '#', to be used 
 *      by .orig only
 */
void assemble_step1(std::istream& asm_file, Program& program) {
    std::string line;
    for(unsigned line_number = 0; getline(asm_file, line); line_number++) {
        if(Instruction instruction(line, line_number); !instruction.empty()) {
           program.emplace_back(instruction);
        }
    }
}

/* STEP 2 = LEXER
 * Checks for every instruction syntax.
 * Does not check for label coherence. But it does fill a LabelMap with label names.
 * Checks for immediate and .ORIG value ranges
 *
 * E.g.
 *    The following instructions...
 *
 *        .END
 *        LD R0 PIPPO
 *        PIPPO .FILL #16
 *        .ORIG #x3000
 *
 *    ...pass this step, even if .END is at the start and .ORIG at the end.
 *
 *    The following instructions, on the other hand...
 *
 *        .ORIG #x3000
 *        PIPPO .FILL #16
 *        ADD R0 R1 #64
 *        AND PIPPO R1 R2
 *        .END
 *
 *    ...do not pass this step since:
 *        - 64 is not a valid immediate value for the ADD instruction.
 *        - PIPPO is not a valid operand for the AND instruction (must be a register).
 *
 * Note that this step does not assign any address to any label. It just puts an entry 
 * into LabelMap for every label declaration encountered. I.e. every LabelMap entry will 
 * have value 0, which is an INVALID address.
 */

void assemble_step2(const Program& program, LabelMap& label_map) {
    for(auto inst : program) {
        constexpr auto REG = TokenType::Register;
        constexpr auto NUM = TokenType::Number;
        constexpr auto LAB = TokenType::Label;
        constexpr auto STR = TokenType::String;

        bool result = false;
        inst.ConsumeLabels(label_map);

        switch(auto& op = inst.front(); op.getType()) {
        case TokenType::Instruction: {
            switch(op.get<OP::Type>()) {
            case OP::ADD: case OP::AND: case OP::XOR:
                result = check_arguments<REG, REG, REG>(inst) ||
                         check_arguments<REG, REG, NUM>(inst);
                break;
            case OP::NOT:
                result = check_arguments<REG, REG>(inst);
                break;
            case OP::JSRR: case OP::JMP:
                result = check_arguments<REG>(inst);
                break;
            case OP::RET: case OP::RTI: case OP::GETC: case OP::OUT: 
            case OP::PUTS: case OP::IN: case OP::PUTSP: case OP::HALT:
                result = check_arguments<>(inst);
                break;
            case OP::LDR: case OP::STR: case OP::LSHF: case OP::RSHFA: case OP::RSHFL:
                result = check_arguments<REG, REG, NUM>(inst);
                break;
            case OP::BR: case OP::BRn: case OP::BRz: case OP::BRp: case OP::BRnz:
            case OP::BRnp: case OP::BRzp: case OP::BRnzp: case OP::JSR:
                result = check_arguments<LAB>(inst);
                break;
            case OP::LD: case OP::LDI: case OP::LEA: case OP::ST: case OP::STI:
                result = check_arguments<REG, LAB>(inst);
                break;
            default:;
            }
            break;
        }
        case TokenType::PseudoOp:
            switch(auto pop_type = op.get<POP::Type>(); pop_type) {
            case POP::ORIG: case POP::FILL:
                result = check_arguments<NUM>(inst);
                break;
            case POP::END:
                result = check_arguments<>(inst);
                break;
            case POP::BLKW:
                result = check_arguments<NUM>(inst) ||
                         check_arguments<NUM, NUM>(inst);
                break;
            case POP::STRINGZ:
                result = check_arguments<STR>(inst);
            default:;
            }
            break;
        default:
            break;
        }
        if (!result) {
            //TODO create proper error type
            throw std::logic_error(error_string(inst.front()));
        }
        if (auto imm = inst.back(), op = inst.front(); 
            imm.getType() == TokenType::Number && !op.checkRange(imm)) {
            //TODO create proper error type
            throw std::logic_error("Wrong range!\n");
        }
    }
}

/* Check for instruction semantics and coherence.
 *    Labels must be declared somewhere (LabelMap filled in previous step).
 *    LabelMap is going to be filled with proper addresses
 *    .ORIG must be at the start (decide whether to issue a warning or an
 *        error if any instruction is written before it)
 *    .END must be at the end (decide whether to issue a warning or an
 *        error if any instruction is written after it)
 *    Out of memory error should be detected here
 *
 * TODO: this step can be merged with step 4
 */

void assemble_step3(const Program& program, LabelMap& label_map) {
    // First instruction MUST be .orig ADDR
    auto first_inst = program.front();
    first_inst.ConsumeLabels(label_map);
    if (first_inst.front() != TokenConsts::ORIG) {
        throw std::logic_error("First instruction must be a valid .ORIG");
    }
    uint16_t address = first_inst.back().get<int>();

    bool end_found = false;
    for(auto inst : program) {
        if (end_found) {
            throw std::logic_error("Everything after .end will be ignored");
        } else if (address > 0xFDFF) {
            throw std::logic_error("Out of memory!");
        }
        else {
            inst.ConsumeLabels(label_map, address);
            if(!inst.empty()) {
                end_found = inst.front() == TokenConsts::END;
                if (const auto& label = inst.back(); 
                    label.getType() == TokenType::Label &&
                    label_map.find(label.get<std::string>()) == label_map.end()) {
                    throw std::logic_error("Label " + label.get<std::string>() + " not found!");
                }
                address+=address_increment(inst);
            }
        }
    }
    if (!end_found) {
        throw std::logic_error("Where is the end?");
    }
}

/* Creates machine code and debug symbols.
 * 
 * The LabelMap is already filled and can be used to generate proper machine code.
 * Label addresses are checked not to be too far from their use.
 * The DebugSymbol are stored **in order** in a vector. 
 *   The order is with respect to the address and to each address corresponds
 *   a file and a line within that file.
 *   Binary search through lower_bound or similar function will work fine.
 */

void assemble_step4(const Program& program, LabelMap& label_map, std::ofstream& out_file, std::ofstream& dbg_file) {

}

void assemble(const std::string& in_filename, const std::string& out_filename, const std::string& dbg_filename) {
    std::ifstream asm_file;
    asm_file.open(in_filename);
    if(asm_file.is_open()) {
        Program program;
        assemble_step1(asm_file, program);

        LabelMap label_map;
        assemble_step2(program, label_map);

        assemble_step3(program, label_map);

        std::ofstream out_file, dbg_file;
        out_file.open(out_filename, std::ios::binary | std::ios::out);
        dbg_file.open(dbg_filename, std::ios::binary | std::ios::out);
        assemble_step4(program, label_map, out_file, dbg_file);
        
        out_file.close();
        dbg_file.close();
        asm_file.close();
    }
}