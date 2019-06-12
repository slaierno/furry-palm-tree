#include "gtest/gtest.h"
#include <cstdint>

/* This is a small playground to test little ASM subroutines.
 * 
 * No memory-related instructions are implemented. They
 * should not be necessary since the goal of this playground
 * is to test little snippets. But they could be easily added
 * if necessary.
 * 
 * No trap and interrupts, of course.
 * 
 * No JMP, JSR and JSRR too. They are...difficult to emulate,
 * since goto does not work that way. There are a couple of
 * options to implement them but they are way more complicated
 * then I want and I prefer to keep things easy here.
 */

uint16_t R0, R1, R2, R3, R4, R5, R6, R7;

enum {
    POS = 0b001, /* P */
    ZRO = 0b010, /* Z */
    NEG = 0b100, /* N */
};
uint8_t CC;

void set_cc(uint16_t R) {
         if(R == 0)     CC = ZRO;
    else if(R < 0x8000) CC = POS;
    else                CC = NEG;
}

#define ADD(DR, SR1, SR2) ({set_cc(DR = SR1 + SR2);})
#define ADDi(DR, SR1, imm5) ({set_cc(DR = SR1 + imm5);})
#define AND(DR, SR1, SR2) ({set_cc(DR = SR1 & SR2);})
#define ANDi(DR, SR1, imm5) ({set_cc(DR = SR1 & imm5);})
#define BR(label) ({goto label;})
#define BRn(label) ({if(CC == NEG) goto label;})
#define BRz(label) ({if(CC == ZRO) goto label;})
#define BRp(label) ({if(CC == POS) goto label;})
#define BRnz(label) ({if(CC != POS) goto label;})
#define BRnp(label) ({if(CC != ZRO) goto label;})
#define BRzp(label) ({if(CC != NEG) goto label;})
#define BRnzp(label) ({goto label;})
#define NOT(DR, SR) ({set_cc(DR = ~SR);})
#define LSHF(DR, SR, imm4) ({set_cc(DR = SR << imm4);})
#define RSHFL(DR, SR, imm4) ({set_cc(DR = SR >> imm4);})
#define RSHFA(DR, SR, imm4) ({set_cc(DR = (SR >> imm4) | (0xFF << (16-imm4)));})

TEST(Operations, Multiplication) {
    uint16_t N = 147, D = 13; //Q=11, R=4
    R1 = N, R2 = D;
    {
        AND(R0, R0, 0);
        ADD(R2, R2, 0);
        LOOP:
            BRz(DONE);
            AND(R3, R2, 0x1);
            BRz(SKIP);
                ADD(R0, R0, R1);
            SKIP:
            LSHF(R1, R1, 1);
            RSHFL(R2, R2, 1);
            BR(LOOP);
        DONE:

        EXPECT_EQ(N*D, R0);
    }
}

TEST(Operations, LongDivision) {
    uint16_t N = 147, D = 13; //Q=11, R=4
    R1 = N, R2 = D;
    {
        AND(R0, R0, 0);
        ADD(R2, R2, 0);
        BRz(ERROR);
        NOT(R3, R2);
        ADD(R3, R3, 1);
        SHIFT:
            LSHF(R3, R3, 1);
            ADD(R4, R1, R3);
            BRp(SHIFT);
            BRz(FOUND);
            RSHFA(R3, R3, 1);
            NEXT:
                LSHF(R0, R0, 1);
                ADD(R4, R1, R3);
                BRn(SKIP);
                FOUND:
                    ADD(R0, R0, 1);
                    ADD(R1, R4, 0);
                    SKIP:
                    ADD(R4, R3, R2);
                    BRz(END);
                    RSHFA(R3, R3, 1);
                    BR(NEXT);
        ERROR:
        ADD(R0, R2, -1);
        END:

        EXPECT_EQ(N/D, R0);
        EXPECT_EQ(N%D, R1);
    }
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
