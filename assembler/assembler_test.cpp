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

class TestInstruction : public TestCommon {};
TEST_F(TestInstruction, RTI) {
    /* TEST INSTRUCTION */
    std::string inst_str = "RTI\n";
    TokenList tokens = tokenize(inst_str);
    EXPECT_EQ(1, tokens.size());

    EXPECT_EQ(TokenType::Instruction, tokens[0].mType);
    EXPECT_EQ(OP::RTI, tokens[0].get<enum OP>());

    EXPECT_NO_THROW(validationStep(tokens));

    EXPECT_EQ(OP_RTI << 12, inst_table[tokens[0].get<enum OP>()](tokens));

    /* TEST BAD INSTRUCTION */
    inst_str = "RTI R0\n";
    tokens = tokenize(inst_str);
    EXPECT_EQ(2, tokens.size());

    EXPECT_EQ(TokenType::Instruction, tokens[0].mType);
    EXPECT_EQ(TokenType::Register,    tokens[1].mType);
    
    EXPECT_EQ(OP::RTI,                tokens[0].get<enum OP>());
    EXPECT_EQ(REG::R0,                tokens[1].get<enum REG>());

    EXPECT_THROW(validationStep(tokens), asm_error::invalid_format);
}

TEST_F(TestInstruction, JSR) {
    label_map.insert(std::pair("PIPPO", 0x3010));
    label_map.insert(std::pair("PLUTO", 0x2FF0));

    /* TEST INSTRUCTION */
    std::string inst_str = "JSR PIPPO\n";
    TokenList tokens = tokenize(inst_str);
    EXPECT_EQ(2, tokens.size());

    EXPECT_EQ(TokenType::Instruction, tokens[0].mType);
    EXPECT_EQ(TokenType::Label,       tokens[1].mType);

    EXPECT_EQ(OP::JSR,                tokens[0].get<enum OP>());
    EXPECT_EQ("PIPPO",                tokens[1].get<std::string>());

    EXPECT_NO_THROW(validationStep(tokens));
    
    EXPECT_EQ(OP_JSR << 12 | 1 << 11 | (0x3010 - 0x3000) & 0x7FF, 
              inst_table[tokens[0].get<enum OP>()](tokens));

    /* TEST INSTRUCTION */
    inst_str = "JSR PLUTO\n";
    tokens = tokenize(inst_str);
    EXPECT_EQ(2, tokens.size());

    EXPECT_EQ(TokenType::Instruction, tokens[0].mType);
    EXPECT_EQ(TokenType::Label,       tokens[1].mType);

    EXPECT_EQ(OP::JSR,                tokens[0].get<enum OP>());
    EXPECT_EQ("PLUTO",                tokens[1].get<std::string>());

    EXPECT_NO_THROW(validationStep(tokens));
    EXPECT_EQ(OP_JSR << 12 | 1 << 11 | (0x2FF0 - 0x3000) & 0x7FF, 
              inst_table[tokens[0].get<enum OP>()](tokens));

    /* TEST BAD INSTRUCTION */
    inst_str = "JSR PAPERINO\n";
    tokens = tokenize(inst_str);
    EXPECT_EQ(2, tokens.size());

    EXPECT_EQ(TokenType::Instruction, tokens[0].mType);
    EXPECT_EQ(TokenType::Label,       tokens[1].mType);

    EXPECT_EQ(OP::JSR,                tokens[0].get<enum OP>());
    EXPECT_EQ("PAPERINO",             tokens[1].get<std::string>());

    EXPECT_NO_THROW(validationStep(tokens));
    EXPECT_THROW(inst_table[tokens[0].get<enum OP>()](tokens), asm_error::label_not_found);

    /* TEST BAD INSTRUCTION */
    inst_str = "JSR\n";
    tokens = tokenize(inst_str);
    EXPECT_EQ(1, tokens.size());

    EXPECT_EQ(TokenType::Instruction, tokens[0].mType);
    EXPECT_EQ(OP::JSR,                tokens[0].get<enum OP>());
    EXPECT_THROW(validationStep(tokens), asm_error::invalid_format);
}

TEST_F(TestInstruction, BR) {
    label_map.insert(std::pair("PIPPO", 0x3010));

    /* TEST BR INSTRUCTIONS */
    std::vector<std::pair<std::string, enum OP>> inst_list {
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

        TokenList tokens = tokenize(inst_str);
        EXPECT_EQ(2, tokens.size());

        EXPECT_EQ(TokenType::Instruction, tokens[0].mType);
        EXPECT_EQ(TokenType::Label,       tokens[1].mType);

        EXPECT_EQ(inst_op,                tokens[0].get<enum OP>());
        EXPECT_EQ("PIPPO",                tokens[1].get<std::string>());

        EXPECT_NO_THROW(validationStep(tokens));
        
        EXPECT_EQ(opcode                   << 12 | 
                  tokens[0].getCondFlags() << 9  | 
                  (0x3010 - 0x3000)         & 0x7FF, 
                  inst_table[tokens[0].get<enum OP>()](tokens));
    }

    /* TEST BAD INSTRUCTION */
    TokenList tokens = tokenize("BR PAPERINO\n");
    EXPECT_EQ(2, tokens.size());

    EXPECT_EQ(TokenType::Instruction, tokens[0].mType);
    EXPECT_EQ(TokenType::Label,       tokens[1].mType);

    EXPECT_EQ(OP::BR,                 tokens[0].get<enum OP>());
    EXPECT_EQ("PAPERINO",             tokens[1].get<std::string>());

    EXPECT_NO_THROW(validationStep(tokens));
    EXPECT_THROW(inst_table[tokens[0].get<enum OP>()](tokens), asm_error::label_not_found);

    /* TEST BAD INSTRUCTION */
    tokens = tokenize("BR\n");
    EXPECT_EQ(1, tokens.size());

    EXPECT_EQ(TokenType::Instruction, tokens[0].mType);
    EXPECT_EQ(OP::BR,                 tokens[0].get<enum OP>());
    EXPECT_THROW(validationStep(tokens), asm_error::invalid_format);
}

TEST_F(TestInstruction, TRAP) {
    /* TEST INSTRUCTION */
    TokenList tokens = tokenize("TRAP #x23\n");
    EXPECT_EQ(2, tokens.size());

    EXPECT_EQ(TokenType::Instruction, tokens[0].mType);
    EXPECT_EQ(TokenType::HexNumber,   tokens[1].mType);

    EXPECT_EQ(OP::TRAP,               tokens[0].get<enum OP>());
    EXPECT_EQ(0x0023,                 tokens[1].get<int>());

    EXPECT_NO_THROW(validationStep(tokens));
    
    EXPECT_EQ(OP_TRAP << 12 | 0x0023, 
              inst_table[tokens[0].get<enum OP>()](tokens));

    /* TEST BAD INSTRUCTION */
    tokens = tokenize("TRAP #35\n");
    EXPECT_EQ(2, tokens.size());

    EXPECT_EQ(TokenType::Instruction, tokens[0].mType);
    EXPECT_EQ(TokenType::Number,      tokens[1].mType);

    EXPECT_EQ(OP::TRAP,               tokens[0].get<enum OP>());
    EXPECT_EQ(0x0023,                 tokens[1].get<int>());

    EXPECT_THROW(validationStep(tokens), asm_error::invalid_format);
    
    EXPECT_EQ(OP_TRAP << 12 | 0x0023, 
              inst_table[tokens[0].get<enum OP>()](tokens));

    /* TEST BAD INSTRUCTION */
    tokens = tokenize("TRAP #x-23\n");
    EXPECT_EQ(2, tokens.size());

    EXPECT_EQ(TokenType::Instruction, tokens[0].mType);
    EXPECT_EQ(TokenType::HexNumber,   tokens[1].mType);

    EXPECT_EQ(OP::TRAP,               tokens[0].get<enum OP>());
    EXPECT_EQ(-35,                    tokens[1].get<int>());

    EXPECT_THROW(validationStep(tokens), asm_error::out_of_range_integer_unsigned);

    /* TEST BAD INSTRUCTION */
    tokens = tokenize("TRAP R0\n");
    EXPECT_EQ(2, tokens.size());

    EXPECT_EQ(TokenType::Instruction, tokens[0].mType);
    EXPECT_EQ(TokenType::Register,    tokens[1].mType);

    EXPECT_EQ(OP::TRAP,               tokens[0].get<enum OP>());
    EXPECT_EQ(REG::R0,                tokens[1].get<enum REG>());

    EXPECT_THROW(validationStep(tokens), asm_error::invalid_format);
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