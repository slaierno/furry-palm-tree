#include "validationSM.hpp"
#include "utils.hpp"
#include "errors.hpp"

static bool origin_found = false;
static bool end_found = false;

template<bool inc>
void constexpr updateInstructionAddress(int n = 1) {
    if(inc) updateInstructionAddress(n);
}

void convertToImmediate(OP::Type& inst, TokenList const& tokens) {
    if(tokens.back().isNumber()) {
        switch(inst) {
        case OP::ADD:
            inst = OP::ADDi;
            break;
        case OP::AND:
            inst = OP::ANDi;
            break;
        case OP::XOR:
            inst = OP::XORi;
            break;
        default:
            break;
        }
    }
}

template<class Error = asm_error::invalid_format>
void finalStateEmpty(TokenList& tokens, Error error = Error()) {
    if(!tokens.empty()) throw error;
}

template<int upper = 0xFFFF, 
         int lower = 0,
         class Error = asm_error::invalid_format>
uint16_t finalStateNumber(TokenList& tokens, Error error = Error()) {
    static_assert(upper <= 0xFFFF && lower >= -0xFFFF);
    if(tokens.empty()) throw error;
    auto front = tokens.front();

    switch(front.getType()) {
    case TokenType::Number:
    case TokenType::HexNumber:
    {
        int number = front.get<int>();
        if(number < lower || number > upper) {
            throw asm_error::out_of_range_integer(lower, upper);
        }
        tokens.pop_front();
        finalStateEmpty(tokens);
        return number;
    }
    default:
        throw error;
    }
}

template<class Error = asm_error::invalid_format>
uint16_t finalStateNumber(TokenList& tokens, OP::Type inst, Error error = Error()) {
    switch(inst) {
    case OP::ADDi:
    case OP::ANDi:
    case OP::XORi:
        return finalStateNumber<0xF, -0x10>(tokens, error);
    case OP::LDR:
    case OP::STR:
        return finalStateNumber<0x1F, -0x20>(tokens, error);
    case OP::LSHF:
    case OP::RSHFA:
    case OP::RSHFL:
        return finalStateNumber<0xF, 0>(tokens, error);
    case OP::TRAP:
        return finalStateNumber<0x25, 0x20>(tokens, error);
    default:
        throw asm_error::unexpected();
    }
}

template<class Error = asm_error::invalid_format>
uint16_t finalStateNumber(TokenList& tokens, POP::Type pop, Error error = Error()) {
    switch(pop) {
    case POP::ORIG:
        return finalStateNumber<0xFDFF, 0x3000>(tokens, error);
    case POP::BLKW:
        return finalStateNumber(tokens, error);
    case POP::FILL:
        //TODO add further checks
        return finalStateNumber(tokens, error);
    default:
        throw asm_error::unexpected();
    }
}

template<class Error = asm_error::invalid_format>
std::string finalStateString(TokenList& tokens, Error error = Error()) {
    if(tokens.empty()) throw error;
    auto front = tokens.front();
    std::string str;

    switch(front.getType()) {
    case TokenType::String:
    {
        str = front.get<std::string>();
        //TODO check string validity
        //TODO add a NULL at the end of the string?
        tokens.pop_front();
        finalStateEmpty(tokens);
        break;
    }
    default:
        throw error;
    }
    return str;
}

template<class Error = asm_error::invalid_format>
void finalStateLabel(TokenList& tokens, Error error = Error()) {
    if(tokens.empty()) throw error;
    auto front = tokens.front();
        
    switch(front.getType()) {
    case TokenType::Label: {
        label_map[front.get<std::string>()]; //inserts a 0 if label_str does not exist yet
        tokens.pop_front();
        finalStateEmpty(tokens);
        break;
    }
    default:
        throw error;
    }
}

template<bool inc>
void statePseudoOp(TokenList& tokens) {
    auto pop = tokens.front().get<POP::Type>();
    tokens.pop_front();

    switch(pop) {
    case POP::STRINGZ:
        updateInstructionAddress<inc>(finalStateString(tokens).length() + 1);
        break;
    case POP::BLKW:
        updateInstructionAddress<inc>(finalStateNumber(tokens, pop));
        break;
    case POP::FILL:
        finalStateNumber(tokens, pop);
        updateInstructionAddress<inc>();
        break;
    default:
        throw asm_error::todo();
    }
}

void stateRegister(TokenList& tokens) {
    if(tokens.empty()) throw asm_error::invalid_format();
    auto front = tokens.front();

    switch(front.getType()) {
    case TokenType::Register:
        tokens.pop_front();
        break;
    default:
        throw asm_error::invalid_format();
    }
}

void stateInstruction(TokenList& tokens) {
    OP::Type inst = tokens.front().get<OP::Type>();
    tokens.pop_front();

    //preprocess to understand if ADD, AND and XOR accept immediate values
    convertToImmediate(inst, tokens);

    switch(inst) {
    case OP::RET:
    case OP::RTI:
        finalStateEmpty(tokens);
        break;
    case OP::BR:
    case OP::BRn:
    case OP::BRz:
    case OP::BRp:
    case OP::BRnz:
    case OP::BRnp:
    case OP::BRzp:
    case OP::BRnzp:
    case OP::JSR:
        finalStateLabel(tokens);
        break;
    case OP::JMP:
    case OP::JSRR:
        stateRegister(tokens);
        finalStateEmpty(tokens);
        break;
    case OP::LD:
    case OP::LDI:
    case OP::ST:
    case OP::STI:
    case OP::LEA:
        stateRegister(tokens);
        finalStateLabel(tokens);
        break;
    case OP::NOT:
        stateRegister(tokens);
        stateRegister(tokens);
        finalStateEmpty(tokens);
        break;
    case OP::ADDi:
    case OP::ANDi:
    case OP::XORi:
    case OP::LDR:
    case OP::STR:
    case OP::LSHF:
    case OP::RSHFA:
    case OP::RSHFL:
        stateRegister(tokens);
        stateRegister(tokens);
        finalStateNumber(tokens, inst);
        break;
    case OP::ADD:
    case OP::AND:
    case OP::XOR:
        stateRegister(tokens);
        stateRegister(tokens);
        stateRegister(tokens);
        finalStateEmpty(tokens);
        break;
    case OP::TRAP:
        finalStateNumber(tokens, inst);
        throw asm_error::trap_inst_disabled();
        break;
    default:
        throw asm_error::unexpected();
    }
}

template<bool inc>
void startState(TokenList& tokens) {
    if(tokens.empty()) return; //noop
    auto front = tokens.front();

    switch(front.getType()) {
    case TokenType::PseudoOp:
        statePseudoOp<inc>(tokens);
        break;
    case TokenType::Trap:
        updateInstructionAddress<inc>();
        tokens.pop_front();
        finalStateEmpty(tokens, asm_error::invalid_trap_call());
        break;
    case TokenType::Instruction:
        updateInstructionAddress<inc>();
        stateInstruction(tokens);
        break;
    default:
        throw asm_error::todo();
    }
}

void startStatePseudoOp(TokenList& tokens) {
    auto pop = tokens.front().get<POP::Type>();
    tokens.pop_front();

    switch(pop) {
    case POP::END:
        if(end_found) throw asm_error::unexpected(); //this should never happen because we skip everything after .END
        end_found = true;
        finalStateEmpty(tokens, asm_error::invalid_pseudo_op(".end shall not be followed by anything else"));
        break;
    case POP::ORIG:
        if(origin_found) throw asm_warning::double_orig_decl();
        origin_found = true;
        inst_address = start_address = finalStateNumber(tokens, pop, asm_error::invalid_pseudo_op(".orig must be followed by an address"));
        break;
    default:
        throw asm_error::todo();
    }
}

template<bool inc>
void validateLineFSMImpl(TokenList& tokens) {
    if(tokens.empty()) return; //noop
    if(inc && end_found) throw asm_warning::inst_after_end(); //ignore since we already found an end
    auto front = tokens.front();
    
    switch(front.getType()) {
    case TokenType::PseudoOp: //only .orig and .end
        startStatePseudoOp(tokens);
        break;
    case TokenType::Label: {
        auto label_str = front.get<std::string>();
        if (0 != label_map[label_str])
            throw asm_error::duplicate_label(front);
        label_map[label_str] = inst_address;
        tokens.pop_front(); 
    } [[fallthrough]];
    default:
        startState<inc>(tokens);
    }

    if(inc && !origin_found) throw asm_warning::inst_before_origin(); //warn since origin has not been declared yet

}
template<>
void validateLineFSM<true>(TokenList tokens) {
    validateLineFSMImpl<true>(tokens);
}
template<>
void validateLineFSM<false>(TokenList tokens) {
    validateLineFSMImpl<false>(tokens);
}

void resetVSM() {
    origin_found = end_found = false;
}