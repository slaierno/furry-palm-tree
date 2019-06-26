#include <iostream>
#include <fstream>
#include "gtest/gtest.h"
#include "Token.hpp"
#include "utils.hpp"
#include "errors.hpp"
#include "validationSM.hpp"

class TestCommon : public ::testing::Test {
protected:
    TestCommon() {
        inst_address = start_address;
        label_map.clear();
        resetVSM();
    };
};

class TestToken : public TestCommon {};
TEST_F(TestToken, Empty) {
    EXPECT_EQ(0, tokenize("").size());
    EXPECT_EQ(0, tokenize(",").size());
    EXPECT_EQ(0, tokenize("\n").size());
    EXPECT_EQ(0, tokenize("\r").size());
    EXPECT_EQ(0, tokenize("\t").size());
    EXPECT_EQ(0, tokenize(";adklfj").size());
}

TEST_F(TestToken, GarbageAndComments) {
    std::string inst_str = "ASD #-123 , ,,, ,RSHFL 124A, #x-ac R1;ADD R0 R0 R0\n";
    TokenList tokens = tokenize(inst_str);
    EXPECT_EQ(6, tokens.size());

    EXPECT_EQ(TokenType::Label,       tokens[0].getType());
    EXPECT_EQ(TokenType::Number,      tokens[1].getType());
    EXPECT_EQ(TokenType::Instruction, tokens[2].getType());
    EXPECT_EQ(TokenType::Label,       tokens[3].getType());
    EXPECT_EQ(TokenType::HexNumber,   tokens[4].getType());
    EXPECT_EQ(TokenType::Register,    tokens[5].getType());

    EXPECT_EQ("ASD",       tokens[0].get<std::string>());
    EXPECT_EQ(-123,        tokens[1].get<int>());
    EXPECT_EQ(OP::RSHFL,   tokens[2].get<OP::Type>());
    EXPECT_EQ("124A",      tokens[3].get<std::string>());
    EXPECT_EQ(-172,        tokens[4].get<int>());
    EXPECT_EQ(REG::R1,     tokens[5].get<REG::Type>());
    
    EXPECT_THROW(validateLineFSM<false>(tokens), asm_error::generic_error);
}

TEST_F(TestToken, PseudoOP) {
    std::string inst_str = ".orig .fill .blkw .stringz .end\n";
    TokenList tokens = tokenize(inst_str);
    EXPECT_EQ(5, tokens.size());

    for(auto const& token : tokens)
        EXPECT_EQ(TokenType::PseudoOp, token.getType());

    EXPECT_EQ(POP::ORIG   , tokens[0].get<POP::Type>());
    EXPECT_EQ(POP::FILL   , tokens[1].get<POP::Type>());
    EXPECT_EQ(POP::BLKW   , tokens[2].get<POP::Type>());
    EXPECT_EQ(POP::STRINGZ, tokens[3].get<POP::Type>());
    EXPECT_EQ(POP::END    , tokens[4].get<POP::Type>());
    
    EXPECT_THROW(validateLineFSM<false>(tokens), asm_error::invalid_pseudo_op);
}

TEST_F(TestToken, Trap) {
    std::string inst_str = "GETC OUT PUTS IN PUTSP HALT\n";
    TokenList tokens = tokenize(inst_str);
    EXPECT_EQ(6, tokens.size());

    for(auto const& token : tokens)
        EXPECT_EQ(TokenType::Trap, token.getType());

    EXPECT_EQ(TRAP::GETC , tokens[0].get<TRAP::Type>());
    EXPECT_EQ(TRAP::OUT  , tokens[1].get<TRAP::Type>());
    EXPECT_EQ(TRAP::PUTS , tokens[2].get<TRAP::Type>());
    EXPECT_EQ(TRAP::IN   , tokens[3].get<TRAP::Type>());
    EXPECT_EQ(TRAP::PUTSP, tokens[4].get<TRAP::Type>());
    EXPECT_EQ(TRAP::HALT , tokens[5].get<TRAP::Type>());
    
    EXPECT_THROW(validateLineFSM<false>(tokens), asm_error::invalid_trap_call);
}

TEST_F(TestToken, BR) {
    /* TEST BR INSTRUCTIONS */
    #define b(type, value) {"BR" #type " PIPPO", OP::BR ## type, value}
    const std::vector<std::tuple<std::string, OP::Type, int>> inst_list {
        b(   , 0b111),
        b(n  , 0b100),
        b(z  , 0b010),
        b(p  , 0b001),
        b(nz , 0b110),
        b(np , 0b101),
        b(zp , 0b011),
        b(nzp, 0b111),
    };
    #undef b
    for(const auto & inst : inst_list) {
        auto inst_str = std::get<0>(inst);
        auto inst_op  = std::get<1>(inst);
        auto cflags   = std::get<2>(inst);

        TokenList tokens = tokenize(inst_str);
        EXPECT_EQ(2, tokens.size());

        EXPECT_EQ(TokenType::Instruction, tokens[0].getType());
        EXPECT_EQ(TokenType::Label,       tokens[1].getType());

        EXPECT_EQ(inst_op,                tokens[0].get<OP::Type>());
        EXPECT_EQ(cflags,                 tokens[0].getCondFlags());
        EXPECT_EQ("PIPPO",                tokens[1].get<std::string>());

        EXPECT_NO_THROW(validateLineFSM<false>(tokens));
    }
}

class TestInstruction : public TestCommon {
public:
    static void testGoodInstruction(const std::string& inst_str, const TokenList& tokens_check, uint16_t inst) {
        TokenList tokens = tokenize(inst_str);
        EXPECT_EQ(tokens_check, tokens) << inst_str;
        EXPECT_NO_THROW(validateLineFSM<false>(tokens)) << inst_str;
        auto temp_inst_copy = inst_str.substr();
        EXPECT_EQ(inst, assembleLine(temp_inst_copy)) << inst_str;
        inst_address--;
    }
    template <typename ErrorType>
    static void testBadInstruction(const std::string& inst_str, const TokenList& tokens_check) {
        TokenList tokens = tokenize(inst_str);
        EXPECT_EQ(tokens_check, tokens) << inst_str;
        EXPECT_THROW(validateLineFSM<false>(tokens), ErrorType) << inst_str;
    }
    template <typename ErrorType>
    static void testBadLabel(const std::string& inst_str, const TokenList& tokens_check) {
        TokenList tokens = tokenize(inst_str);
        EXPECT_EQ(tokens_check, tokens) << inst_str;
        EXPECT_NO_THROW(validateLineFSM<false>(tokens)) << inst_str;
        EXPECT_THROW(inst_table[tokens[0].get<OP::Type>()](tokens), ErrorType) << inst_str;
    }
};
TEST_F(TestInstruction, RTI) {
    /* TEST INSTRUCTION */
    testGoodInstruction("RTI\n",
        {{TokenType::Instruction, OP::RTI}},
        OP_RTI << 12);

    /* TEST BAD INSTRUCTION */
    testBadInstruction<asm_error::invalid_format>(
        "RTI R0\n",
        {{TokenType::Instruction, OP::RTI},
         {TokenType::Register   , REG::R0}}
    );
}

TEST_F(TestInstruction, JSR) {
    label_map.emplace("PIPPO", 0x3010);
    label_map.emplace("PLUTO", 0x2FF0);

    /* TEST INSTRUCTION */
    testGoodInstruction(
        "JSR PIPPO\n",
        {{TokenType::Instruction, OP::JSR},
         {TokenType::Label      , "PIPPO"}},
        OP_JSR << 12 | 1 << 11 | ((0x3010 - 0x3000) & 0x7FF)
    );
    testGoodInstruction(
        "JSR PLUTO\n",
        {{TokenType::Instruction, OP::JSR},
         {TokenType::Label      , "PLUTO"}},
        OP_JSR << 12 | 1 << 11 | ((0x2FF0 - 0x3000) & 0x7FF)
    );

    /* TEST BAD INSTRUCTION */
    testBadLabel<asm_error::label_not_found>(
        "JSR PAPERINO\n",
        {{TokenType::Instruction, OP::JSR},
         {TokenType::Label      , "PAPERINO"}}
    );

    testBadInstruction<asm_error::invalid_format>(
        "JSR\n",
        {{TokenType::Instruction, OP::JSR}}
    );
}

TEST_F(TestInstruction, BR) {
    label_map.insert(std::pair("PIPPO", 0x3010));

    /* TEST BR INSTRUCTIONS */
    #define b(type, value) {"BR" #type " PIPPO\n", OP::BR ## type}
    const std::vector<std::pair<std::string, OP::Type>> inst_list {
        b(   , 0b111),
        b(n  , 0b100),
        b(z  , 0b010),
        b(p  , 0b001),
        b(nz , 0b110),
        b(np , 0b101),
        b(zp , 0b011),
        b(nzp, 0b111),
    };
    for(const auto & inst : inst_list) {
        auto inst_str = inst.first;
        auto inst_op  = inst.second;
        auto opcode   = opEnumToOpcodeMap[inst_op];

        testGoodInstruction(
            inst_str,
            {{TokenType::Instruction, inst_op},
             {TokenType::Label      , "PIPPO"}},
            opcode                 << 12  | 
            brToCondFlag(inst_op)  << 9   | 
            ((0x3010 - 0x3000)     & 0x7FF)
        );
    }
    /* TEST BAD INSTRUCTION */
    testBadLabel<asm_error::label_not_found>(
        "BR PAPERINO\n",
        {{TokenType::Instruction, OP::BR},
         {TokenType::Label      , "PAPERINO"}}
    );

    testBadInstruction<asm_error::invalid_format>(
        "BR\n",
        {{TokenType::Instruction, OP::BR}}
    );
}

TEST_F(TestInstruction, TRAP) {
    /* TEST TRAP INSTRUCTIONS */
    #define t(trap) {#trap, TRAP::trap, trapEnumToOpcodeMap[TRAP::trap]}
    const std::vector<std::tuple<std::string, TRAP::Type, uint16_t>> inst_list {
        t(GETC),
        t(OUT),
        t(PUTS),
        t(IN),
        t(PUTSP),
        t(HALT),
    };
    #undef t
    for(auto const& inst : inst_list) testGoodInstruction(
        std::get<0>(inst),
        {{TokenType::Trap, std::get<1>(inst)}},
        OP_TRAP << 12  | std::get<2>(inst)
    );

    /* TEST BAD INSTRUCTION */
    testBadInstruction<asm_error::trap_inst_disabled>(
        "TRAP #x23\n",
        {{TokenType::Instruction, OP::TRAP},
         {TokenType::HexNumber  , 0x0023}}
    );

    testBadInstruction<asm_error::out_of_range_integer>(
        "TRAP #x-23\n",
        {{TokenType::Instruction, OP::TRAP},
         {TokenType::HexNumber  , -35}}
    );    

    testBadInstruction<asm_error::invalid_format>(
        "TRAP R0\n",
        {{TokenType::Instruction, OP::TRAP},
         {TokenType::Register   , REG::R0}}
    );
}

TEST_F(TestInstruction, JSRR) {
    /* TEST INSTRUCTION */
    testGoodInstruction(
        "JSRR R2\n",
        {{TokenType::Instruction, OP::JSRR},
         {TokenType::Register   , REG::R2}},
        OP_JSR << 12 | R_R2 << 6
    );

    /* TEST BAD INSTRUCTION */
    testBadInstruction<asm_error::invalid_format>(
        "JSRR #x23\n",
        {{TokenType::Instruction, OP::JSRR},
         {TokenType::HexNumber  , 0x0023}}
    );
}

TEST_F(TestInstruction, JMP_RET) {
    /* TEST INSTRUCTION */
    testGoodInstruction(
        "JMP R2\n",
        {{TokenType::Instruction, OP::JMP},
         {TokenType::Register   , REG::R2}},
        OP_JMP << 12 | R_R2 << 6
    );
    testGoodInstruction(
        "RET\n",
        {{TokenType::Instruction, OP::RET}},
        OP_JMP << 12 | R_R7 << 6
    );

    /* TEST BAD INSTRUCTION */
    testBadInstruction<asm_error::invalid_format>(
        "JMP #x23\n",
        {{TokenType::Instruction, OP::JMP},
         {TokenType::HexNumber  , 0x0023}}
    );
    testBadInstruction<asm_error::invalid_format>(
        "RET #x23\n",
        {{TokenType::Instruction, OP::RET},
         {TokenType::HexNumber  , 0x0023}}
    );
}

TEST_F(TestInstruction, LD_ST_LDI_STI_LEA) {
    label_map.insert(std::pair("PIPPO", 0x3010));
    #define o(N) {#N, OP::N, opEnumToOpcodeMap[OP::N]}
    const std::vector<std::tuple<std::string, OP::Type, uint16_t>> inst_list {
        o(LD), o(ST), o(LDI), o(STI), o(LEA),
    };
    #undef o
    for(auto const& inst : inst_list) {
        /* TEST INSTRUCTION */
        testGoodInstruction(
            std::get<0>(inst) + " R1 PIPPO\n",
            {{TokenType::Instruction, std::get<1>(inst)},
             {TokenType::Register   , REG::R1},
             {TokenType::Label      , "PIPPO"}},
            std::get<2>(inst) << 12 | R_R1 << 9 | ((0x3010 - 0x3000) & 0x1FF)
        );
        /* TEST BAD INSTRUCTION */
        testBadInstruction<asm_error::invalid_format>(
            std::get<0>(inst) + " #x23\n",
            {{TokenType::Instruction, std::get<1>(inst)},
            {TokenType::HexNumber  , 0x0023}}
        );
    }
}

TEST_F(TestInstruction, NOT) {
    /* TEST INSTRUCTION */
    testGoodInstruction(
        "NOT R2 R3\n",
        {{TokenType::Instruction, OP::NOT},
         {TokenType::Register   , REG::R2},
         {TokenType::Register   , REG::R3}},
        OP_NOT << 12 | R_R2 << 9 | R_R3 << 6 | 0x3F
    );

    /* TEST BAD INSTRUCTION */
    testBadInstruction<asm_error::invalid_format>(
        "NOT R2\n",
        {{TokenType::Instruction, OP::NOT},
         {TokenType::Register   , REG::R2}}
    );
}

TEST_F(TestInstruction, ADD_AND_XOR) {
    #define o(N) {#N, OP::N, opEnumToOpcodeMap[OP::N]}
    const std::vector<std::tuple<std::string, OP::Type, uint16_t>> inst_list {
        o(ADD), o(AND), o(XOR)
    };
    #undef o
    for(auto const& inst : inst_list) {
        /* TEST INSTRUCTION */
        /* REG,REG */
        testGoodInstruction(
            std::get<0>(inst) + " R2 R3 R4\n",
            {{TokenType::Instruction, std::get<1>(inst)},
            {TokenType::Register    , REG::R2},
            {TokenType::Register    , REG::R3},
            {TokenType::Register    , REG::R4}},
            std::get<2>(inst) << 12 | R_R2 << 9 | R_R3 << 6 | R_R4
        );

        /* REG,IMM5 */
        testGoodInstruction(
            std::get<0>(inst) + " R2 R3 #5\n",
            {{TokenType::Instruction, std::get<1>(inst)},
            {TokenType::Register    , REG::R2},
            {TokenType::Register    , REG::R3},
            {TokenType::Number      , 5}},
            std::get<2>(inst) << 12 | R_R2 << 9 | R_R3 << 6 | 1 << 5 | 5
        );
        testGoodInstruction(
            std::get<0>(inst) + " R2 R3 #-16\n",
            {{TokenType::Instruction, std::get<1>(inst)},
            {TokenType::Register    , REG::R2},
            {TokenType::Register    , REG::R3},
            {TokenType::Number      , -16}},
            std::get<2>(inst) << 12 | R_R2 << 9 | R_R3 << 6 | 1 << 5 | 0x10
        );
        testGoodInstruction(
            std::get<0>(inst) + " R2 R3 #x-F\n",
            {{TokenType::Instruction, std::get<1>(inst)},
            {TokenType::Register    , REG::R2},
            {TokenType::Register    , REG::R3},
            {TokenType::HexNumber   , -15}},
            std::get<2>(inst) << 12 | R_R2 << 9 | R_R3 << 6 | 1 << 5 | 0x11
        );

        /* TEST BAD INSTRUCTION */
        testBadInstruction<asm_error::out_of_range_integer>(
            std::get<0>(inst) + " R2 R3 #x15\n",
            {{TokenType::Instruction, std::get<1>(inst)},
            {TokenType::Register    , REG::R2},
            {TokenType::Register    , REG::R3},
            {TokenType::HexNumber   , 0x15}}
        );
        testBadInstruction<asm_error::out_of_range_integer>(
            std::get<0>(inst) + " R2 R3 #16\n",
            {{TokenType::Instruction, std::get<1>(inst)},
            {TokenType::Register    , REG::R2},
            {TokenType::Register    , REG::R3},
            {TokenType::Number      , 16}}
        );
        testBadInstruction<asm_error::out_of_range_integer>(
            std::get<0>(inst) + " R2 R3 #-17\n",
            {{TokenType::Instruction, std::get<1>(inst)},
            {TokenType::Register    , REG::R2},
            {TokenType::Register    , REG::R3},
            {TokenType::Number      , -17}}
        );
    }
}

TEST_F(TestInstruction, LDR_STR) {
    #define o(N) {#N, OP::N, opEnumToOpcodeMap[OP::N]}
    const std::vector<std::tuple<std::string, OP::Type, uint16_t>> inst_list {
        o(LDR), o(STR)
    };
    #undef o
    for(auto const& inst : inst_list) {
        /* TEST INSTRUCTION */
        testGoodInstruction(
            std::get<0>(inst) + " R2 R3 #x-11\n",
            {{TokenType::Instruction, std::get<1>(inst)},
            {TokenType::Register    , REG::R2},
            {TokenType::Register    , REG::R3},
            {TokenType::HexNumber   , -17}},
            std::get<2>(inst) << 12 | R_R2 << 9 | R_R3 << 6 | (-17 & 0x3F)
        );

        /* TEST BAD INSTRUCTION */
        testBadInstruction<asm_error::invalid_format>(
            std::get<0>(inst) + " R2 R3 PAPERINO\n",
            {{TokenType::Instruction, std::get<1>(inst)},
            {TokenType::Register    , REG::R2},
            {TokenType::Register    , REG::R3},
            {TokenType::Label      , "PAPERINO"}}
        );
        testBadInstruction<asm_error::out_of_range_integer>(
            std::get<0>(inst) + " R2 R3 #x20\n",
            {{TokenType::Instruction, std::get<1>(inst)},
            {TokenType::Register    , REG::R2},
            {TokenType::Register    , REG::R3},
            {TokenType::HexNumber   , 32}}
        );
        testBadInstruction<asm_error::out_of_range_integer>(
            std::get<0>(inst) + " R2 R3 #-33\n",
            {{TokenType::Instruction, std::get<1>(inst)},
            {TokenType::Register    , REG::R2},
            {TokenType::Register    , REG::R3},
            {TokenType::Number      , -33}}
        );
    }
}

TEST_F(TestInstruction, SHF) {
    #define o(N) {#N, OP::N, opEnumToOpcodeMap[OP::N]}
    const std::vector<std::tuple<std::string, OP::Type, uint16_t>> inst_list {
        o(LSHF), o(RSHFL), o(RSHFA)
    };
    #undef o    

    for(auto const& inst : inst_list) {
        auto flag = [](decltype(inst)& i)->uint16_t { 
            return ((std::get<1>(i) == OP::RSHFA) << 1) | 
                    (std::get<1>(i) != OP::LSHF);
        };
        /* TEST INSTRUCTION */
        testGoodInstruction(
            std::get<0>(inst) + " R2 R3 #5\n",
            {{TokenType::Instruction, std::get<1>(inst)},
            {TokenType::Register    , REG::R2},
            {TokenType::Register    , REG::R3},
            {TokenType::Number      , 5}},
            std::get<2>(inst) << 12 | R_R2 << 9 | R_R3 << 6 | flag(inst) << 4 | 5
        );
        testGoodInstruction(
            std::get<0>(inst) + " R2 R3 #xF\n",
            {{TokenType::Instruction, std::get<1>(inst)},
            {TokenType::Register    , REG::R2},
            {TokenType::Register    , REG::R3},
            {TokenType::HexNumber   , 15}},
            std::get<2>(inst) << 12 | R_R2 << 9 | R_R3 << 6 | flag(inst) << 4 | 0xF
        );

        /* TEST BAD INSTRUCTION */
        testBadInstruction<asm_error::out_of_range_integer>(
            std::get<0>(inst) + " R2 R3 #x-F\n",
            {{TokenType::Instruction, std::get<1>(inst)},
            {TokenType::Register    , REG::R2},
            {TokenType::Register    , REG::R3},
            {TokenType::HexNumber   , -15}}
        );
        testBadInstruction<asm_error::out_of_range_integer>(
            std::get<0>(inst) + " R2 R3 #16\n",
            {{TokenType::Instruction, std::get<1>(inst)},
            {TokenType::Register    , REG::R2},
            {TokenType::Register    , REG::R3},
            {TokenType::Number      , 16}}
        );
        testBadInstruction<asm_error::out_of_range_integer>(
            std::get<0>(inst) + " R2 R3 #-17\n",
            {{TokenType::Instruction, std::get<1>(inst)},
            {TokenType::Register    , REG::R2},
            {TokenType::Register    , REG::R3},
            {TokenType::Number      , -17}}
        );
    }
}

class TestAssembly : public TestCommon {};

TEST_F(TestAssembly, Labels) {
    std::vector<std::string> inst_list = {
        ".orig #x4000",
        "PIPPO\n",        //x4000
        "JSR PLUTO\n",    //x4000
        "PAPERINO\n",     //x4001
        "RTI\n",          //x4001
        "PLUTO\n",        //x4002
        "JSR PAPERINO\n", //x4002
    };
    for(auto inst_str : inst_list)
        EXPECT_NO_THROW(validateLine(inst_str)) << inst_str;

    EXPECT_THROW(validateLine(".orig #x5000"), asm_warning::double_orig_decl);
    EXPECT_NO_THROW(validateLine(".end"));
    EXPECT_THROW(validateLine("TOTAL GARBAGE"), asm_warning::inst_after_end);

    EXPECT_NE(label_map.end(), label_map.find("PIPPO"));
    EXPECT_NE(label_map.end(), label_map.find("PAPERINO"));
    EXPECT_NE(label_map.end(), label_map.find("PLUTO"));
    EXPECT_EQ(0x4000,          label_map.find("PIPPO")->second);
    EXPECT_EQ(0x4001,          label_map.find("PAPERINO")->second);
    EXPECT_EQ(0x4002,          label_map.find("PLUTO")->second);
    
    inst_address = start_address;
    ASSERT_EQ(0x4000, inst_address);

    std::vector<uint16_t> code_list;
    for(auto inst_str : inst_list) EXPECT_NO_THROW({
        uint16_t inst = assembleLine(inst_str);
        if (inst != 0) code_list.push_back(inst);
    }) << inst_str;
    EXPECT_EQ(3, code_list.size());
    EXPECT_EQ(OP_JSR << 12 | 1 << 11 | ((0x4002 - 0x4000) & 0x7FF), code_list[0]);
    EXPECT_EQ(OP_RTI << 12,                                         code_list[1]);
    EXPECT_EQ(OP_JSR << 12 | 1 << 11 | ((0x4001 - 0x4002) & 0x7FF), code_list[2]);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
