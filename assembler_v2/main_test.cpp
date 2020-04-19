#include <iostream>
#include <fstream>
#include "gtest/gtest.h"
#include "assembler_test.hpp"

class TestAssembler_v2 : public ::testing::Test {};

TEST_F(TestAssembler_v2, TokenConstructor) {
    #define ASSERT_INSTRUCTION(STR, INST) \
        ASSERT_EQ(TokenType::Instruction, Token(STR).getType()); \
        ASSERT_EQ(OP::INST, Token(STR).get<OP::Type>());

    ASSERT_INSTRUCTION("RET"  , RET);
    ASSERT_INSTRUCTION("RTI"  , RTI);
    ASSERT_INSTRUCTION("JSR"  , JSR);
    ASSERT_INSTRUCTION("bR"   , BR);
    ASSERT_INSTRUCTION("BrN"  , BRn);
    ASSERT_INSTRUCTION("BRz"  , BRz);
    ASSERT_INSTRUCTION("BRp"  , BRp);
    ASSERT_INSTRUCTION("BRnz" , BRnz);
    ASSERT_INSTRUCTION("BRzp" , BRzp);
    ASSERT_INSTRUCTION("BRnp" , BRnp);
    ASSERT_INSTRUCTION("BRnzp", BRnzp);
    ASSERT_INSTRUCTION("TRAP" , TRAP);
    ASSERT_INSTRUCTION("JSRR" , JSRR);
    ASSERT_INSTRUCTION("JMP"  , JMP);
    ASSERT_INSTRUCTION("LD"   , LD);
    ASSERT_INSTRUCTION("ST"   , ST);
    ASSERT_INSTRUCTION("LDI"  , LDI);
    ASSERT_INSTRUCTION("STI"  , STI);
    ASSERT_INSTRUCTION("LEA"  , LEA);
    ASSERT_INSTRUCTION("NOT"  , NOT);
    ASSERT_INSTRUCTION("ADD"  , ADD);
    ASSERT_INSTRUCTION("AND"  , AND);
    ASSERT_INSTRUCTION("LDR"  , LDR);
    ASSERT_INSTRUCTION("STR"  , STR);
    ASSERT_INSTRUCTION("LSHF" , LSHF);
    ASSERT_INSTRUCTION("RSHFL", RSHFL);
    ASSERT_INSTRUCTION("RSHFA", RSHFA);
    ASSERT_INSTRUCTION("xOr"  , XOR);
    ASSERT_INSTRUCTION("geTC" , GETC);
    ASSERT_INSTRUCTION("Out"  , OUT);
    ASSERT_INSTRUCTION("PutS" , PUTS);
    ASSERT_INSTRUCTION("IN"   , IN);
    ASSERT_INSTRUCTION("putsp", PUTSP);
    ASSERT_INSTRUCTION("halt" , HALT);

    #undef ASSERT_INSTRUCTION

    #define ASSERT_PSEUDOOP(STR, INST) \
        ASSERT_EQ(TokenType::PseudoOp, Token(STR).getType()); \
        ASSERT_EQ(POP::INST, Token(STR).get<POP::Type>());

    ASSERT_PSEUDOOP(".ORIG"   , ORIG);
    ASSERT_PSEUDOOP(".fill"   , FILL);
    ASSERT_PSEUDOOP(".blkw"   , BLKW);
    ASSERT_PSEUDOOP(".strINgz", STRINGZ);
    ASSERT_PSEUDOOP(".eND"    , END);
    
    #undef ASSERT_PSEUDOOP

    #define ASSERT_REG(R, STR) \
        ASSERT_EQ(TokenType::Register, Token(STR).getType()); \
        ASSERT_EQ(REG::R, Token(STR).get<REG::Type>());

    ASSERT_REG(R0, "r0");
    ASSERT_REG(R0, "R0");
    ASSERT_REG(R1, "r1");
    ASSERT_REG(R1, "R1");
    ASSERT_REG(R2, "r2");
    ASSERT_REG(R2, "R2");
    ASSERT_REG(R3, "r3");
    ASSERT_REG(R3, "R3");
    ASSERT_REG(R4, "r4");
    ASSERT_REG(R4, "R4");
    ASSERT_REG(R5, "r5");
    ASSERT_REG(R5, "R5");
    ASSERT_REG(R6, "r6");
    ASSERT_REG(R6, "R6");
    ASSERT_REG(R7, "r7");
    ASSERT_REG(R7, "R7");

    #undef ASSERT_REG

    #define ASSERT_LABEL(STR) \
        ASSERT_EQ(TokenType::Label, Token(STR).getType()); \
        ASSERT_EQ(STR, Token(STR).get<std::string>());

    ASSERT_LABEL("Test");
    ASSERT_LABEL("Test2");
    ASSERT_LABEL("_Test");
    ASSERT_LABEL("_TeST__ABCD");
    ASSERT_LABEL("R1ABC");
    ASSERT_LABEL("r2ABC");
    ASSERT_LABEL("0TEST");

    #undef ASSERT_LABEL

    #define ASSERT_NUMBER(NUM, STR) \
        ASSERT_EQ(TokenType::Number, Token(STR).getType()); \
        ASSERT_EQ(NUM, Token(STR).get<int>());

    ASSERT_NUMBER(145, "#145");
    ASSERT_NUMBER(-145, "#-145");
    ASSERT_NUMBER(234, "xEA");
    ASSERT_NUMBER(0xEA, "#234");

    #undef ASSERT_NUMBER

    //Note add quotes outside quotes in order to make a string that
    //  include those quotes and let the Token constructor recognize
    //  the content as a string.
    auto stringify = [](std::string str) {
        std::stringstream ss;
        ss << "\"" << str << "\"";
        return ss.str();
    };
    #define ASSERT_STRING(STR) ({\
        ASSERT_EQ(TokenType::String, Token(stringify(STR)).getType()); \
        ASSERT_EQ(STR, Token(stringify(STR)).get<std::string>()); })

    //Every valid label can be a valid string also, if enclosed in quotes.
    ASSERT_STRING("Test");
    ASSERT_STRING("Test2");
    ASSERT_STRING("_Test");
    ASSERT_STRING("_TeST__ABCD");
    //But string can also have non alphanumeric(+underscore) characters
    //Every following test would fail without quotes
    ASSERT_STRING("r8");
    ASSERT_STRING("R8");
    ASSERT_STRING("R1ABC");
    ASSERT_STRING("r2ABC");
    ASSERT_STRING("0918");
    ASSERT_STRING("0TEST");
    ASSERT_STRING("Invàlid");
    ASSERT_STRING("Tést");
    ASSERT_STRING("Te`st");
    ASSERT_STRING("Te'st");
    
    //And of course any valid token can be a string
    //OPs, POPs and REGs
    #define ENUM_MACRO(X) ASSERT_STRING(#X);
    OP_TYPES
    POP_TYPES
    REG_TYPES
    #undef ENUM_MACRO
    //But numbers too
    ASSERT_STRING("#145");
    ASSERT_STRING("#-145");
    ASSERT_STRING("xEA");
    ASSERT_STRING("#234");

    #undef ASSERT_STRING

    #define ASSERT_UNDEFINED(STR) \
        ASSERT_EQ(TokenType::Undefined, Token(STR).getType());
    
    ASSERT_UNDEFINED("r8");
    ASSERT_UNDEFINED("R8");
    ASSERT_UNDEFINED("0987");
    ASSERT_UNDEFINED("123");
    ASSERT_UNDEFINED("Invàlid");
    ASSERT_UNDEFINED("Tést");
    ASSERT_UNDEFINED("Te`st");
    ASSERT_UNDEFINED("Te'st");
    ASSERT_UNDEFINED("\"Test");
    ASSERT_UNDEFINED("Test\"");
    
    #undef ASSERT_UNDEFINED
}

TEST_F(TestAssembler_v2, EqualOperator) {
    {
        Token token1("ADD");
        Token token2("add");
        Token token3("AND");
        ASSERT_EQ(token1, token2);
        ASSERT_NE(token1, token3);
    }
    {
        Token token1(".stringz");
        Token token2(".STRINGZ");
        Token token3(".blkw");
        ASSERT_EQ(token1, token2);
        ASSERT_NE(token1, token3);
    }
    {
        Token token1("R0");
        Token token2("r0");
        Token token3("R6");
        ASSERT_EQ(token1, token2);
        ASSERT_NE(token1, token3);
    }
    {
        Token token1("#234");
        Token token2("xea");
        Token token3("#-234");
        ASSERT_EQ(token1, token2);
        ASSERT_NE(token1, token3);
    }
    {
        Token token1("PIPPO");
        Token token3("PLUTO");
        Token token2("PIPPO");
        Token token4("pippo");
        ASSERT_EQ(token1, token2);
        ASSERT_NE(token1, token3);
        ASSERT_NE(token1, token4);
    }
    {
        Token token1("\"PIPPO\"");
        Token token2("\"PIPPO\"");
        Token token3("PIPPO");
        ASSERT_EQ(token1, token2);
        ASSERT_NE(token1, token3);
        //Even though token1 and token3 are different by type, they are equal by value.
        ASSERT_EQ(token1.get<std::string>(), token3.get<std::string>());
    }
}

TEST_F(TestAssembler_v2, Constants) {
    EXPECT_EQ(TokenConsts::ORIG, Token(".ORIG"));
    EXPECT_EQ(TokenConsts::END, Token(".END"));
}

TEST_F(TestAssembler_v2, LineTrimmer) {
    std::string garbage = "   \t \t   AND PIPPO #12093800 \tx938ff  \"this is a stringòàùè+\" R0 R1 \t  \t ; THIS IS    GOìnG t0 be r0 r1 r2 ignored";
    ASSERT_EQ("AND PIPPO #12093800 \tx938ff  \"this is a stringòàùè+\" R0 R1", trim_line(garbage));
    
    //Check if everything works with empty lines
    std::string empty_line = "";
    ASSERT_EQ("", trim_line(empty_line));
}

TEST_F(TestAssembler_v2, Instruction) {
    const std::string garbage = "   \t \t   AND PIPPO #12093800 \tx938ff  \"this is a stringòàùè+\" R0 R1 \t  \t ; THIS IS    GOìnG t0 be r0 r1 r2 ignored";
    Instruction instruction(garbage, 711);
    ASSERT_EQ(garbage, instruction.GetOGString());
    ASSERT_EQ(711, instruction.GetLineNumber());
    ASSERT_EQ(7, instruction.size());
    ASSERT_EQ(Token("and"), instruction[0]);
    ASSERT_EQ(Token("PIPPO"), instruction[1]);
    ASSERT_NE(Token("pippo"), instruction[1]); //label are case sensitive
    ASSERT_EQ(Token("xb88968"), instruction[2]); //checking equivalence between 0xB88968 and 12093800
    ASSERT_EQ(Token("#604415"), instruction[3]);  //checking equivalence between 604415 and 0x938FF
    ASSERT_EQ(Token("\"this is a stringòàùè+\""), instruction[4]);
    ASSERT_EQ(Token("r0"), instruction[5]);
    ASSERT_EQ(Token("r1"), instruction[6]);

    const std::string single = ".end";
    instruction = Instruction(single, 712);
    ASSERT_EQ(single, instruction.GetOGString());
    ASSERT_EQ(712, instruction.GetLineNumber());
    ASSERT_EQ(1, instruction.size());
    ASSERT_EQ(Token(".end"), instruction[0]);
}

TEST_F(TestAssembler_v2, AssemblerStep1) {
    std::stringstream ss;
    ss << "ADD PIPPO #342" << std::endl;
    ss << "PLUTO R2 x42" << std::endl;
    ss << "" << std::endl;
    ss << "     " << std::endl;
    ss << ".stringz \"THIS is a   String\"   ;This is a comment" << std::endl;
    ss << "; this is only a comment, skip me" << std::endl;
    ss << ".end" << std::endl;
    ss << ".orig x0030" << std:: endl;
    ss << "" << std::endl;

    Program program;
    assemble_step1(ss, program);
    ASSERT_EQ(5, program.size());
    ASSERT_EQ(3, program[0].size());
        ASSERT_EQ(Token("add"), program[0][0]);
        ASSERT_EQ(Token("PIPPO"), program[0][1]);
        ASSERT_EQ(Token("#342"), program[0][2]);
    ASSERT_EQ(3, program[1].size());
        ASSERT_EQ(Token("PLUTO"), program[1][0]);
        ASSERT_EQ(Token("r2"), program[1][1]);
        ASSERT_EQ(Token("x42"), program[1][2]);
    ASSERT_EQ(2, program[2].size());
        ASSERT_EQ(Token(".STRINGZ"), program[2][0]);
        ASSERT_EQ(Token("\"THIS is a   String\""), program[2][1]);
    ASSERT_EQ(1, program[3].size());
        ASSERT_EQ(Token(".END"), program[3][0]);
    ASSERT_EQ(2, program[4].size());
        ASSERT_EQ(Token(".ORIG"), program[4][0]);
        ASSERT_EQ(Token("x0030"), program[4][1]);
}

TEST_F(TestAssembler_v2, AssemblerStep2Utils) {
    LabelMap label_map;
    Instruction inst("PIPPO PLUTO PAPERINO ADD x3000 PIPPO");

    /* The two following asserts are somehow broken, because the first token is skipped.
       Usually, the first token should be TokenType::Instruction or TokenType::PseudoOp,
        but here we test instruction_check() for Instrution with a different first token.
     */
    ASSERT_FALSE((check_arguments<TokenType::Label, TokenType::Number>(inst)));
    ASSERT_TRUE((check_arguments<TokenType::Label, TokenType::Label, TokenType::Instruction, TokenType::Number, TokenType::Label>(inst)));

    ASSERT_TRUE(inst.ConsumeLabels(label_map));
    ASSERT_FALSE(Instruction("").ConsumeLabels(label_map));
    ASSERT_EQ(3, label_map.size());
    ASSERT_NE(label_map.end(), label_map.find("PIPPO"));
    ASSERT_NE(label_map.end(), label_map.find("PLUTO"));
    ASSERT_NE(label_map.end(), label_map.find("PAPERINO"));

    ASSERT_FALSE((check_arguments<TokenType::Number>(inst)));
    ASSERT_TRUE((check_arguments<TokenType::Number, TokenType::Label>(inst)));

    inst = Instruction("RET ;test a RET instruction");
    ASSERT_FALSE((check_arguments<TokenType::Instruction>(inst)));
    ASSERT_TRUE((check_arguments<>(inst)));
}

//TODO expand failure tests
TEST_F(TestAssembler_v2, AssemblerStep2False) {
    std::stringstream ss;
    ss << "ADD PIPPO #342" << std::endl;
    ss << "PLUTO R2 x42" << std::endl;
    ss << "" << std::endl;
    ss << "     " << std::endl;
    ss << ".stringz \"THIS is a   String\"   ;This is a comment" << std::endl;
    ss << "; this is only a comment, skip me" << std::endl;
    ss << ".end" << std::endl;
    ss << ".orig x0030" << std:: endl;
    ss << "" << std::endl;

    Program program;
    LabelMap label_map;
    assemble_step1(ss, program);
    //TODO this should throw meaningful exceptions
    ASSERT_THROW(assemble_step2(program, label_map), std::logic_error);
}

#define ASSERT_MAP_PRESENT(MAP, KEY) ASSERT_NE((MAP).end(), (MAP).find(KEY))
#define EXPECT_MAP_PRESENT(MAP, KEY) EXPECT_NE((MAP).end(), (MAP).find(KEY))
#define ASSERT_MAP_ABSENT(MAP, KEY)  ASSERT_EQ((MAP).end(), (MAP).find(KEY))
#define EXPECT_MAP_ABSENT(MAP, KEY)  EXPECT_EQ((MAP).end(), (MAP).find(KEY))

TEST_F(TestAssembler_v2, AssemblerStep2) {
    std::stringstream ss;
    ss << "MYLABEL .end              ;COMMENT" << std::endl;
    ss << "MYLABEL ADD   R0 R1 #2    ;COMMENT" << std::endl;
    ss << "MYLABEL ADD   R0 R1 R2    ;COMMENT" << std::endl;
    ss << "MYLABEL AND   R0 R1 #2    ;COMMENT" << std::endl;
    ss << "MYLABEL AND   R0 R1 R2    ;COMMENT" << std::endl;
    ss << "MYLABEL XOR   R0 R1 #2    ;COMMENT" << std::endl;
    ss << "MYLABEL XOR   R0 R1 R2    ;COMMENT" << std::endl;
    ss << "MYLABEL LSHF  R0 R1 #2    ;COMMENT" << std::endl;
    ss << "MYLABEL RSHFL R0 R1 #2    ;COMMENT" << std::endl;
    ss << "MYLABEL RSHFA R0 R1 #2    ;COMMENT" << std::endl;
    ss << "MYLABEL NOT   R0 R1       ;COMMENT" << std::endl;
    ss << "MYLABEL JSRR  R0          ;COMMENT" << std::endl;
    ss << "MYLABEL JMP   R0          ;COMMENT" << std::endl;
    ss << "MYLABEL RET               ;COMMENT" << std::endl;
    ss << "MYLABEL RTI               ;COMMENT" << std::endl;
    ss << "MYLABEL GETC              ;COMMENT" << std::endl;
    ss << "MYLABEL OUT               ;COMMENT" << std::endl;
    ss << "MYLABEL PUTS              ;COMMENT" << std::endl;
    ss << "MYLABEL IN                ;COMMENT" << std::endl;
    ss << "MYLABEL PUTSP             ;COMMENT" << std::endl;
    ss << "MYLABEL HALT              ;COMMENT" << std::endl;
    ss << "MYLABEL LDR   R0 R1 x1f   ;COMMENT" << std::endl;
    ss << "MYLABEL STR   R0 R1 x1f   ;COMMENT" << std::endl;
    ss << "MYLABEL LD    R0 PIPPO    ;COMMENT" << std::endl;
    ss << "MYLABEL LDI   R0 PIPPO    ;COMMENT" << std::endl;
    ss << "MYLABEL LEA   R0 PIPPO    ;COMMENT" << std::endl;
    ss << "MYLABEL ST    R0 PIPPO    ;COMMENT" << std::endl;
    ss << "MYLABEL STI   R0 PIPPO    ;COMMENT" << std::endl;
    ss << "MYLABEL BR    PIPPO       ;COMMENT" << std::endl;
    ss << "MYLABEL BRn   PIPPO       ;COMMENT" << std::endl;
    ss << "MYLABEL BRz   PIPPO       ;COMMENT" << std::endl;
    ss << "MYLABEL BRp   PIPPO       ;COMMENT" << std::endl;
    ss << "MYLABEL BRnz  PIPPO       ;COMMENT" << std::endl;
    ss << "MYLABEL BRnp  PIPPO       ;COMMENT" << std::endl;
    ss << "MYLABEL BRzp  PIPPO       ;COMMENT" << std::endl;
    ss << "MYLABEL BRnzp PIPPO       ;COMMENT" << std::endl;
    ss << "MYLABEL JSR   PIPPO       ;COMMENT" << std::endl;
    ss << "MYLABEL .fill xffff       ;COMMENT" << std::endl;
    ss << "MYLABEL .blkw x10 xffff   ;COMMENT" << std::endl;
    ss << "MYLABEL .blkw x10         ;COMMENT" << std::endl;
    ss << "MYLABEL .stringz \"HELLO\";COMMENT" << std::endl;
    ss << "MYLABEL .orig x3000       ;COMMENT" << std:: endl;

    Program program;
    LabelMap label_map;
    assemble_step1(ss, program);
    assemble_step2(program, label_map);

    ASSERT_MAP_PRESENT(label_map, "MYLABEL");
    ASSERT_MAP_ABSENT(label_map, "PIPPO");
}

//TODO expand failure tests
TEST_F(TestAssembler_v2, AssemblerStep3False) {
    {
        Program program;
        LabelMap label_map;
        std::stringstream ss;
        ss << "ADD R0 R0 #12" << std::endl;

        assemble_step1(ss, program);
        assemble_step2(program, label_map);
        //TODO this should throw meaningful exceptions
        ASSERT_THROW(assemble_step3(program, label_map), std::logic_error);
    }
    {
        Program program;
        LabelMap label_map;
        std::stringstream ss;
        ss << ".orig x3000" << std::endl;
        ss << "ADD R0 R0 #12" << std::endl;

        assemble_step1(ss, program);
        assemble_step2(program, label_map);
        //TODO this should throw meaningful exceptions
        ASSERT_THROW(assemble_step3(program, label_map), std::logic_error);
    }
    {
        Program program;
        LabelMap label_map;
        std::stringstream ss;
        ss << ".orig x3000" << std::endl;
        ss << "ADD R0 R0 #12" << std::endl;
        ss << "LEA R0 PIPPO" << std::endl;
        ss << ".end" << std::endl;

        assemble_step1(ss, program);
        assemble_step2(program, label_map);
        //TODO this should throw meaningful exceptions
        ASSERT_THROW(assemble_step3(program, label_map), std::logic_error);
    }
    {
        Program program;
        LabelMap label_map;
        std::stringstream ss;
        ss << ".orig x3000" << std::endl;
        ss << "ADD R0 R0 #12" << std::endl;
        ss << "LEA R0 PIPPO" << std::endl;
        ss << "PIPPO .stringz \"HELLO\"" << std::endl;
        ss << "PIPPO .stringz \"WORLD\"" << std::endl;
        ss << ".end" << std::endl;

        assemble_step1(ss, program);
        assemble_step2(program, label_map);
        //TODO this should throw meaningful exceptions
        ASSERT_THROW(assemble_step3(program, label_map), std::logic_error);
    }
}

TEST_F(TestAssembler_v2, AssemblerStep3) {
    Program program;
    LabelMap label_map;
    std::stringstream ss;
    ss << ".orig x3000" << std::endl;
    ss << "ADD R0 R0 #12" << std::endl;
    ss << "LEA R0 PIPPO" << std::endl;
    ss << "PIPPO .stringz \"HELLO\"" << std::endl;
    ss << ".fill xff00" << std::endl;
    ss << "PLUTO .stringz \"WORLD\"" << std::endl;
    ss << ".blkw #12 xff77" << std::endl;
    ss << "PAPERINO AND R1 R1 #0" << std::endl;
    ss << ".end" << std::endl;

    assemble_step1(ss, program);
    assemble_step2(program, label_map);
    assemble_step3(program, label_map);
    ASSERT_MAP_PRESENT(label_map, "PIPPO");
    ASSERT_MAP_PRESENT(label_map, "PLUTO");
    ASSERT_MAP_PRESENT(label_map, "PAPERINO");
    ASSERT_EQ(0x3002, label_map["PIPPO"]);
    ASSERT_EQ(0x3009, label_map["PLUTO"]);
    ASSERT_EQ(0x301B, label_map["PAPERINO"]);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
