#include "gtest/gtest.h"
#include <cstdint>

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

TEST(Division, LongDivision) {
    uint16_t N = 147, D = 13; //Q=11, R=4
    R1 = N, R2 = D;
    {
        R0 = 0;
    back:
        R2 <<= 1;
        if(R1 > R2) goto back;
        if(R1 == R2) goto next;
        R2 >>= 1;
    next:
        R3 = R1 - R2;
        R0 <<= 1;
        if(R3 < 0x8000) {
            R0 |= 1;
            R1 = R3;
        }
        if(R2 == D) goto end;
        R2 >>= 1;
        goto next;
    end:
        EXPECT_EQ(N/D, R0);
    }

    R1 = N, R2 = D;
    {
        R0 = R0 & 0;
        R3 = ~R2;
        R3++; //R3 = -R2
    BACK:
        R3 = R3 << 1;
        set_cc(R4 = R1 + R3);
        if(CC == POS) goto BACK;
        if(CC == ZRO) goto FOUND;
        R3 = R3 >> 1 | 0x8000; //RSHFA
    NEXT:
        R0 = R0 << 1;
        set_cc(R4 = R1 + R3); //instruction clone
        if (CC == NEG) goto SKIP;
    FOUND:
        R0 = R0 + 1;
        R1 = R4 + 0;
    SKIP:
        set_cc(R4 = R3 + R2);
        if(CC == ZRO) goto END;
        R3 = R3 >> 1 | 0x8000; //RSHFA
        goto NEXT;
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
