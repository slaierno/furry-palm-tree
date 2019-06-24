#include <array>
#include "gtest/gtest.h"
#include "lc3-hw.hpp"
#include "lc3-video.hpp"

// The fixture for testing class Foo.
class TestOperand : public ::testing::Test {
public:
    uint16_t instr, op, r0, r1, r2, imm5, old_pc, offset;
    enum { PC_START = 0x3000 };

protected:
    TestOperand() : r0(R_R0), r1(R_R1), r2(R_R2) {}
};

TEST_F(TestOperand, OP_ADDRegister) {
    // TEST OP_ADD REG
    op = OP_ADD;
    reg[r0] = 0;
    reg[r1] = 5;
    reg[r2] = 8;
    instr = op << 12 | r0 << 9 | r1 << 6 | r2;
    op_table[op](instr);
    EXPECT_NE(reg[r1] + r2 ,reg[r0]);
    EXPECT_EQ(reg[r1] + reg[r2], reg[r0]);
    EXPECT_EQ(FL_POS, reg[R_COND]);
}

TEST_F(TestOperand, OP_ADDimm5) {
    op = OP_ADD;
    imm5 = -6;
    reg[r0] = 0;
    reg[r1] = 4;
    reg[imm5 & 0x7] = 7;
    instr = op << 12 | r0 << 9 | r1 << 6 | 1 << 5 | (imm5 & 0x1F);
    op_table[op](instr);
    EXPECT_NE(reg[r1] + reg[imm5 & 0x7], reg[r0]);
    EXPECT_EQ((uint16_t) (reg[r1] + imm5), reg[r0]);
    EXPECT_EQ(FL_NEG, reg[R_COND]);
}

TEST_F(TestOperand, OP_ADDimm5FL_ZRO) {
    op = OP_ADD;
    imm5 = -6;
    reg[r0] = 0;
    reg[r1] = 6;
    reg[imm5 & 0x7] = 7;
    instr = op << 12 | r0 << 9 | r1 << 6 | 1 << 5 | (imm5 & 0x1F);
    op_table[op](instr);
    EXPECT_NE(reg[r1] + reg[imm5 & 0x7], reg[r0]);
    EXPECT_EQ((uint16_t) (reg[r1] + imm5), reg[r0]);
    EXPECT_EQ(FL_ZRO, reg[R_COND]);
}

TEST_F(TestOperand, OP_ANDRegister) {
    op = OP_AND;
    reg[r0] = 0;
    reg[r1] = 0b1101011101111010;
    reg[r2] = 0b1011000101101011;
    instr = op << 12 | r0 << 9 | r1 << 6 | r2;
    op_table[op](instr);
    EXPECT_NE((reg[r1] & r2     ), reg[r0]);
    EXPECT_EQ((reg[r1] & reg[r2]), reg[r0]);
    EXPECT_EQ(FL_NEG, reg[R_COND]);
}

TEST_F(TestOperand, OP_ANDimm5) {
    op = OP_AND;
    imm5 = 0xFFF2; //-2
    reg[r0] = 0;
    reg[r1] = 0b0101011101111010;
    reg[imm5 & 0x7] = 0b1011000101101011;
    instr = op << 12 | r0 << 9 | r1 << 6 | 1 << 5 | (imm5 & 0x1F);
    op_table[op](instr);
    EXPECT_NE((reg[r1] & reg[imm5 & 0x7]), reg[r0]);
    EXPECT_EQ((reg[r1] & imm5   ), reg[r0]);
    EXPECT_EQ(FL_POS, reg[R_COND]);
}

TEST_F(TestOperand, OP_NOT) {
    op = OP_NOT;
    reg[r0] = 0;
    reg[r1] = 0b1101011101111010;
    instr = op << 12 | r0 << 9 | r1 << 6 | 0x3F;
    op_table[op](instr);
    EXPECT_EQ((uint16_t)~reg[r1], (uint16_t)reg[r0]);
    EXPECT_EQ(FL_POS, reg[R_COND]);
}

TEST_F(TestOperand, OP_XORRegister) {
    op = OP_XOR;
    reg[r0] = 0;
    reg[r1] = 0b1101011101111010;
    reg[r2] = 0b1011000101101011;
    instr = op << 12 | r0 << 9 | r1 << 6 | r2;
    op_table[op](instr);
    EXPECT_NE((reg[r1] ^ r2     ), reg[r0]);
    EXPECT_EQ((reg[r1] ^ reg[r2]), reg[r0]);
    EXPECT_EQ(FL_POS, reg[R_COND]);
}

TEST_F(TestOperand, OP_XORimm5) {
    op = OP_XOR;
    imm5 = 0xFFF2; //-2
    reg[r0] = 0;
    reg[r1]         = 0b0101011101111010;
    reg[imm5 & 0x7] = 0b1011000101101011;
    instr = op << 12 | r0 << 9 | r1 << 6 | 1 << 5 | (imm5 & 0x1F);
    op_table[op](instr);
    EXPECT_NE((reg[r1] ^ reg[imm5 & 0x7]), reg[r0]);
    EXPECT_EQ((reg[r1] ^ imm5   ), reg[r0]);
    EXPECT_EQ(FL_NEG, reg[R_COND]);
}

TEST_F(TestOperand, OP_BR) {
op = OP_BR;
offset = -4;
for(r0 = 0b000; r0 < 0b1000; r0++) {
    for(uint16_t flag = 0b001; flag < 0b1000; flag <<= 1) {
        old_pc = reg[R_PC];
        reg[R_COND] = flag;
        instr = op << 12 | r0 << 9 | (offset & 0x1FF);
        op_table[op](instr);
        if(flag & r0)
            EXPECT_EQ((uint16_t)(old_pc + offset), (uint16_t)reg[R_PC]);
        else
            EXPECT_EQ(old_pc, reg[R_PC]);
    }
}
r0 = R_R0;
}

TEST_F(TestOperand, OP_JMP) {
    op = OP_JMP;
    reg[r1] = -16;
    old_pc = reg[R_PC];
    instr = op << 12 | r1 << 6;
    op_table[op](instr);
    EXPECT_EQ((uint16_t)reg[r1], (uint16_t)reg[R_PC]);
}

TEST_F(TestOperand, OP_JSR) {
    op = OP_JSR;
    old_pc = reg[R_PC];
    offset = -16;
    instr = op << 12 | 1 << 11 | (offset & 0x7FF);
    op_table[op](instr);
    EXPECT_EQ(old_pc, reg[R_R7]);
    EXPECT_EQ((uint16_t)(old_pc + offset), (uint16_t)reg[R_PC]);
}

TEST_F(TestOperand, OP_JSRR) {
    op = OP_JSR;
    reg[r1] = 0x3000;
    old_pc = reg[R_PC];
    instr = op << 12 | r1 << 6;
    op_table[op](instr);
    EXPECT_EQ(old_pc, reg[R_R7]);
    EXPECT_EQ((uint16_t)reg[r1], (uint16_t)reg[R_PC]);
}

TEST_F(TestOperand, OP_LD) {
    op = OP_LD;
    offset = -16;
    memory[(uint16_t)(reg[R_PC] + offset)] = 0xABCD;
    instr = op << 12 | r0 << 9 | (offset & 0x1FF);
    op_table[op](instr);
    EXPECT_EQ(0xABCD, reg[r0]);
}

TEST_F(TestOperand, OP_LDI) {
    op = OP_LDI;
    offset = -16;
    memory[(uint16_t)(reg[R_PC] + offset)] = 0x4000;
    memory[0x4000] = 0xDCBA;
    instr = op << 12 | r0 << 9 | (offset & 0x1FF);
    op_table[op](instr);
    EXPECT_EQ(0xDCBA, reg[r0]);
}

TEST_F(TestOperand, OP_LDR) {
    op = OP_LDR;
    reg[r1] = 0x5010;
    offset = -16;
    memory[0x5000] = 0xB00B;
    instr = op << 12 | r0 << 9 | r1 << 6 | (offset & 0x3F);
    op_table[op](instr);
    EXPECT_EQ(0xB00B, reg[r0]);
}

TEST_F(TestOperand, OP_LEA) {
    op = OP_LEA;
    offset = -16;
    instr = op << 12 | r0 << 9 | (offset & 0x1FF);
    op_table[op](instr);
    EXPECT_EQ((uint16_t)(reg[R_PC] + offset), reg[r0]);
}

TEST_F(TestOperand, OP_ST) {
    op = OP_ST;
    offset = -16;
    reg[r0] = 0xBABE;
    instr = op << 12 | r0 << 9 | (offset & 0x1FF);
    op_table[op](instr);
    EXPECT_EQ(0xBABE, memory[(uint16_t)(reg[R_PC] + offset)]);
}

TEST_F(TestOperand, OP_STI) {
    op = OP_STI;
    offset = -16;
    reg[r0] = 0xCAFE;
    memory[(uint16_t)(reg[R_PC] + offset)] = 0x4000;
    instr = op << 12 | r0 << 9 | (offset & 0x1FF);
    op_table[op](instr);
    EXPECT_EQ(0xCAFE, memory[0x4000]);
}

TEST_F(TestOperand, OP_STR) {
    op = OP_STR;
    offset = -16;
    reg[r1] = 0x6010;
    reg[r0] = 0x1EE7;
    instr = op << 12 | r0 << 9 | r1 << 6 | (offset & 0x3F);
    op_table[op](instr);
    EXPECT_EQ(0x1EE7, memory[0x6000]);
}

TEST_F(TestOperand, OP_LSHF) {
    op = OP_SHF;
    offset = 0b000100; //|L|L|$4
    reg[r1] = 0xEC10;
    reg[r0] = 0x1EE7;
    instr = op << 12 | r0 << 9 | r1 << 6 | (offset & 0x3F);
    op_table[op](instr);
    EXPECT_EQ(0xC100, reg[r0]);
}

TEST_F(TestOperand, OP_RSHFL) {
    op = OP_SHF;
    offset = 0b010100; //|L|R|$4
    reg[r1] = 0xEC10;
    reg[r0] = 0x1EE7;
    instr = op << 12 | r0 << 9 | r1 << 6 | (offset & 0x3F);
    op_table[op](instr);
    EXPECT_EQ(0x0EC1, reg[r0]);
}

TEST_F(TestOperand, OP_RSHFA) {
    op = OP_SHF;
    offset = 0b110100; //|A|R|$4
    reg[r1] = 0xEC10;
    reg[r0] = 0x1EE7;
    instr = op << 12 | r0 << 9 | r1 << 6 | (offset & 0x3F);
    op_table[op](instr);
    EXPECT_EQ(0xFEC1, reg[r0]);
}

class TestVideo : public ::testing::Test {
public:
    LC3Screen lc3s;
    const uint16_t base_address = LC3Screen::BASE_VIDEO_MEMORY;
};

TEST_F(TestVideo, Little) {
    uint16_t address = base_address;

    const std::vector<uint16_t> colors = {
        0x0123, 0x4567, 0x89AB, 0xCDEF,
        0x1032, 0x5476, 0x98BA,
    };
    const int COLOR_NUM = colors.size();

    int color_index = 0;
    uint16_t counter = 0; //8 bit
    for (int x = 0; x < HEIGTH; x++) {
        if (x % 8 == 0) {
            for (int y = 0; y < WIDTH; y+=8) {
                mem_write(address++, colors[color_index]);
                ++color_index %= COLOR_NUM;
            }
        }
        for (int y = 0; y < WIDTH; y+=16) {
            mem_write88(address++, counter, counter+1);
            counter+=2;
        }
    }
    ASSERT_EQ(0xFD88, address);

    lc3s.mem_to_screen();

    color_index = 0;
    counter = 0;
    for(int y = 0; y < HEIGTH; y++) {
        color_index = ((y / 8) * TWIDTH) % COLOR_NUM;
        for(int x = 0; x < WIDTH; x+=8) {
            uint16_t color = colors[color_index];
            for(int p = 0; p < 8; p++) {
                uint8_t pix_color = lc3s.screen[y*WIDTH + (x+p)];
                uint8_t expected_fg = (counter >> (7 - p)) & 0x1;
                uint8_t expected_color = (expected_fg) ? FG(color) : BG(color);
                ASSERT_EQ(pix_color, expected_color) << "(y,x/p) = " << y << "," << x << "/" << p;
            }
            counter++;
            ++color_index %= COLOR_NUM;
        }
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}