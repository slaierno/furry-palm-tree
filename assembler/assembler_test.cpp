#include <iostream>
#include <fstream>
#include "gtest/gtest.h"
#include "Token.hpp"
#include "utils.hpp"
#include "errors.hpp"

class TestCommon : public ::testing::Test {
protected:
    const uint16_t start_address;
    TestCommon() : start_address(inst_address) {};
    ~TestCommon() { 
        inst_address = start_address; 
        label_map.clear();
    };
};

class TestToken : public TestCommon {};
TEST_F(TestToken, GarbageAndComments) {
    std::string inst_str = "ASD #-123 , ,,, ,RSHFL 124A, #x-ac R1;ADD R0 R0 R0\n";
    TokenList tokens = tokenize(inst_str);
    EXPECT_EQ(6, tokens.size());

    EXPECT_EQ(TokenType::Label,       tokens[0].mType);
    EXPECT_EQ(TokenType::Number,      tokens[1].mType);
    EXPECT_EQ(TokenType::Instruction, tokens[2].mType);
    EXPECT_EQ(TokenType::Label,       tokens[3].mType);
    EXPECT_EQ(TokenType::HexNumber,   tokens[4].mType);
    EXPECT_EQ(TokenType::Register,    tokens[5].mType);

    EXPECT_EQ("ASD",       tokens[0].get<std::string>());
    EXPECT_EQ(-123,        tokens[1].get<int>());
    EXPECT_EQ(OP::RSHFL,   tokens[2].get<enum OP>());
    EXPECT_EQ("124A",      tokens[3].get<std::string>());
    EXPECT_EQ(-172,        tokens[4].get<int>());
    EXPECT_EQ(REG::R1,     tokens[5].get<enum REG>());
    
    EXPECT_THROW(validationStep(tokens), asm_error::invalid_label_decl);
}

TEST_F(TestToken, BR) {
    /* TEST BR INSTRUCTIONS */
    std::vector<std::tuple<std::string, enum OP, int>> inst_list {
        {"BR PIPPO\n",    OP::BR,    0b111},
        {"BRn PIPPO\n",   OP::BRn,   0b100},
        {"BRz PIPPO\n",   OP::BRz,   0b010},
        {"BRp PIPPO\n",   OP::BRp,   0b001},
        {"BRnz PIPPO\n",  OP::BRnz,  0b110},
        {"BRnp PIPPO\n",  OP::BRnp,  0b101},
        {"BRzp PIPPO\n",  OP::BRzp,  0b011},
        {"BRnzp PIPPO\n", OP::BRnzp, 0b111},
    };
    for(const auto & inst : inst_list) {
        auto inst_str = std::get<0>(inst);
        auto inst_op  = std::get<1>(inst);
        auto cflags   = std::get<2>(inst);
        auto opcode   = op_map.find(inst_op)->second;

        TokenList tokens = tokenize(inst_str);
        EXPECT_EQ(2, tokens.size());

        EXPECT_EQ(TokenType::Instruction, tokens[0].mType);
        EXPECT_EQ(TokenType::Label,       tokens[1].mType);

        EXPECT_EQ(inst_op,                tokens[0].get<enum OP>());
        EXPECT_EQ(cflags,                 tokens[0].getCondFlags());
        EXPECT_EQ("PIPPO",                tokens[1].get<std::string>());

        EXPECT_NO_THROW(validationStep(tokens));
    }
}

class TestInstruction : public TestCommon {
public:
    static void testGoodInstruction(const std::string& inst_str, const TokenList& tokens_check, uint16_t inst) {
        TokenList tokens = tokenize(inst_str);
        EXPECT_EQ(tokens_check, tokens);
        EXPECT_NO_THROW(validationStep(tokens));
        EXPECT_EQ(inst, inst_table[tokens[0].get<enum OP>()](tokens)) << inst_str;
    }
    template <typename ErrorType>
    static void testBadInstruction(const std::string& inst_str, const TokenList& tokens_check) {
        TokenList tokens = tokenize(inst_str);
        EXPECT_EQ(tokens_check, tokens);
        EXPECT_THROW(validationStep(tokens), ErrorType) << inst_str;
    }
    template <typename ErrorType>
    static void testBadLabel(const std::string& inst_str, const TokenList& tokens_check) {
        TokenList tokens = tokenize(inst_str);
        EXPECT_EQ(tokens_check, tokens);
        EXPECT_NO_THROW(validationStep(tokens));
        EXPECT_THROW(inst_table[tokens[0].get<enum OP>()](tokens), ErrorType) << inst_str;
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
    label_map.insert(std::pair("PIPPO", 0x3010));
    label_map.insert(std::pair("PLUTO", 0x2FF0));

    /* TEST INSTRUCTION */
    testGoodInstruction(
        "JSR PIPPO\n",
        {{TokenType::Instruction, OP::JSR},
         {TokenType::Label      , "PIPPO"}},
        OP_JSR << 12 | 1 << 11 | (0x3010 - 0x3000) & 0x7FF
    );

    testGoodInstruction(
        "JSR PLUTO\n",
        {{TokenType::Instruction, OP::JSR},
         {TokenType::Label      , "PLUTO"}},
        OP_JSR << 12 | 1 << 11 | (0x2FF0 - 0x3000) & 0x7FF
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
    const std::vector<std::pair<std::string, enum OP>> inst_list {
        {"BR PIPPO\n",    OP::BR},
        {"BRn PIPPO\n",   OP::BRn},
        {"BRz PIPPO\n",   OP::BRz},
        {"BRp PIPPO\n",   OP::BRp},
        {"BRnz PIPPO\n",  OP::BRnz},
        {"BRnp PIPPO\n",  OP::BRnp},
        {"BRzp PIPPO\n",  OP::BRzp},
        {"BRnzp PIPPO\n", OP::BRnzp},
    };
    for(const auto & inst : inst_list) {
        auto inst_str = inst.first;
        auto inst_op  = inst.second;
        auto opcode   = op_map.find(inst_op)->second;

        testGoodInstruction(
            inst_str,
            {{TokenType::Instruction, inst_op},
             {TokenType::Label      , "PIPPO"}},
            opcode                          << 12 | 
            Token::getCondFlags(inst_op)    << 9  | 
            (0x3010 - 0x3000)                & 0x7FF
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
    /* TEST INSTRUCTION */
    testGoodInstruction(
        "TRAP #x23\n",
        {{TokenType::Instruction, OP::TRAP},
         {TokenType::HexNumber  , 0x0023}},
        OP_TRAP << 12 | 0x0023
    );

    /* TEST BAD INSTRUCTION */
    testBadInstruction<asm_error::invalid_format>(
        "TRAP #35\n",
        {{TokenType::Instruction, OP::TRAP},
         {TokenType::Number     , 35}}
    );    
    // EXPECT_EQ(OP_TRAP << 12 | 0x0023, 
    //           inst_table[tokens[0].get<enum OP>()](tokens));

    testBadInstruction<asm_error::out_of_range_integer_unsigned>(
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
}

class TestAssembly : public TestCommon {};

TEST_F(TestAssembly, Labels) {
    uint16_t start_address = inst_address;
    std::vector<std::string> inst_list = {
        "PIPPO\n",        //x3000
        "JSR PLUTO\n",    //x3000
        "PAPERINO\n",     //x3001
        "RTI\n",          //x3001
        "PLUTO\n",        //x3002
        "JSR PAPERINO\n", //x3002
    };
    for(auto inst_str : inst_list)
        EXPECT_NO_THROW(validateLine(inst_str));

    EXPECT_NE(label_map.end(), label_map.find("PIPPO"));
    EXPECT_NE(label_map.end(), label_map.find("PAPERINO"));
    EXPECT_NE(label_map.end(), label_map.find("PLUTO"));
    EXPECT_EQ(0x3000,          label_map.find("PIPPO")->second);
    EXPECT_EQ(0x3001,          label_map.find("PAPERINO")->second);
    EXPECT_EQ(0x3002,          label_map.find("PLUTO")->second);
    
    inst_address = start_address;
    std::vector<uint16_t> code_list;
    for(auto inst_str : inst_list) {
        EXPECT_NO_THROW({
            uint16_t inst = assembleLine(inst_str);
            if (inst != 0) code_list.push_back(inst);
            });
    }
    EXPECT_EQ(3, code_list.size());
    EXPECT_EQ(OP_JSR << 12 | 1 << 11 | (0x3002 - 0x3000) & 0x7FF, code_list[0]);
    EXPECT_EQ(OP_RTI << 12,                                       code_list[1]);
    EXPECT_EQ(OP_JSR << 12 | 1 << 11 | (0x3001 - 0x3002) & 0x7FF, code_list[2]);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}