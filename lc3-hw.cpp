#include "lc3-hw.hpp"
#include <iostream>

uint16_t reg[R_COUNT];
uint16_t PC_START = 0x3000;

void update_flags(uint16_t r) {
    if(reg[r] == 0) {
        reg[R_COND] = FL_ZRO;
    } else if (reg[r] >> 15) {
        reg[R_COND] = FL_NEG;
    } else {
        reg[R_COND] = FL_POS;
    }
}

void read_image_file(FILE* file)
{
    /* the origin tells us where in memory to place the image */
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);
    PC_START = origin;

    /* we know the maximum file size so we only need one fread */
    uint16_t max_read = UINT16_MAX - origin;
    uint16_t* p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    /* swap to little endian */
    while (read-- > 0)
    {
        *p = swap16(*p);
        ++p;
    }
}

int read_image(const char* image_path)
{
    FILE* file = fopen(image_path, "rb");
    if (!file) { return 0; };
    read_image_file(file);
    fclose(file);
    return 1;
}

#ifdef _WIN32

HANDLE hStdin;    
DWORD fdwSaveOldMode;

void ErrorExit (char* lpszMessage) 
{ 
    std::cerr << lpszMessage << std::endl; 

    // Restore input mode on exit.

    SetConsoleMode(hStdin, fdwSaveOldMode);

    ExitProcess(0); 
}

void disable_input_buffering()
{
    // Get the standard input handle.
    hStdin = GetStdHandle(STD_INPUT_HANDLE); 
        
    // Save the current input mode, to be restored on exit.
    GetConsoleMode(hStdin, &fdwSaveOldMode); 

    SetConsoleMode(hStdin, fdwSaveOldMode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));
}

void restore_input_buffering()
{
    SetConsoleMode(hStdin, fdwSaveOldMode);
}

#else

struct termios original_tio;

void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

#endif

void handle_interrupt(int signal)
{
    restore_input_buffering();
    std::cout << std::endl;
    exit(-2);
}

int running = 1;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsequence-point"
template <const unsigned op> void exec(uint16_t instr) {
    const uint16_t opbit = 1 << op;
    uint16_t r0, r1, op2, offset;
    if(0x0100 & opbit) { //RTI
        if((reg[R_PSR] >> 15) == 0) {
            reg[R_PC] = mem_read(reg[R_R6]); //R6 is the SSP
            reg[R_R6]++;
            uint16_t temp = mem_read(reg[R_R6]);
            reg[R_R6]++;
            reg[R_PSR] = temp;
        } else {
            //TODO Privilege mode exception
            abort();
        }
    }
    if(0x6EEF & opbit) r0 = (instr >> 9) & 0x7; //also work as COND for BR
    if(0x32F2 & opbit) r1 = (instr >> 6) & 0x7;
    if(0x1000 & opbit) reg[R_PC] = reg[r1]; //JMP
    if(0x0222 & opbit) { //ADD, AND, XOR
                        op2 = ((instr >> 5) & 1) ? //imm_flag
                            sign_extend(instr & 0x1F, 5): //op2 = imm5
                            op2 = reg[(instr & 0x7)]; //op2 = reg[r2]
    }
    if(0x2000 & opbit) { //SHF/RES
                        op2 = instr & 0xF; //op2 = imm4
                        if((instr >> 4) & 1) { //D flag
                            //RSHF
                            if((instr >> 5) & 1) { //A flag
                                //RSHFA
#ifndef _ANDROID
                                asm("movw %%bx, %%ax;"
                                    "sarw %%cl, %%ax;"
                                    :"=a"(reg[r0])
                                    : "b"(reg[r1]), "c"(op2)
                                    );
#else
				reg[r0] = (int16_t) reg[r1] / (1 << op2);
#endif
                            } else {
                                //RSHFL
#ifndef _ANDROID
                                asm("movw %%bx, %%ax;"
                                    "shrw %%cl, %%ax;"
                                    :"=a"(reg[r0])
                                    : "b"(reg[r1]), "c"(op2)
                                    );
#else
				reg[r0] = reg[r1] >> op2;
#endif
                            }
                        } else {
                            //LSHF
                            reg[r0] = reg[r1] << op2;
                        }
    }
    if(0x0002 & opbit) reg[r0] = reg[r1] + op2; //ADD
    if(0x0020 & opbit) reg[r0] = reg[r1] & op2; //AND
    if(0x0200 & opbit) reg[r0] = reg[r1] ^ op2; //NOT
    if(0x4C0D & opbit) offset = reg[R_PC] + sign_extend(instr & 0x1FF, 9); //offset = PC+PCoffset9
    if(0x00C0 & opbit) offset = reg[r1] + sign_extend(instr & 0x3F, 6); //offset = R1+Offset6
    if(0x0001 & opbit) if (r0 & reg[R_COND]) reg[R_PC] = offset; //BR
    if(0x0044 & opbit) reg[r0] = mem_read(offset); //LD & LDR
    if(0x0088 & opbit) mem_write(offset, reg[r0]); //ST & STR
    if(0x0010 & opbit) { //JSR(R)
                                        reg[R_R7] = reg[R_PC]; 
                                        reg[R_PC] = ((instr >> 11) & 1) ? 
                                            (reg[R_PC] + sign_extend(instr & 0x7FF, 11)) : //JSR
                                            reg[r1]; //JSRR
    }
    if(0x0400 & opbit) reg[r0] = mem_read(mem_read(offset)); //LDI
    if(0x0800 & opbit) mem_write(mem_read(offset), reg[r0]); //STI
    if(0x4000 & opbit) reg[r0] = offset; //LEA
    if(0x6666 & opbit) update_flags(r0);
    if(0x8000 & opbit) { //TRAP
        switch (instr & 0xFF)
        {
            case TRAP_GETC:
                {
                    reg[R_R0] = (uint16_t) input_buffer.pop_or_wait();
                }
                break;
            case TRAP_OUT:
                {
                    std::cout << (char)reg[R_R0];
                }
                break;
            case TRAP_PUTS:
                {
                    uint16_t start_address = reg[R_R0];
                    uint16_t* c = &memory[start_address];
                    while(*c)
                    {
                        std::cout << (char)*c;
                        ++c;
                    }
                }
                break;
            case TRAP_IN:
                {
                    std::cout << "Enter a character: ";
                    char c = input_buffer.pop_or_wait();
                    std::cout << c;
                    reg[R_R0] = (uint16_t) c;
                }
                break;
            case TRAP_PUTSP:
                {
                    uint16_t start_address = reg[R_R0];
                    uint16_t* c = &memory[start_address];
                    while(*c)
                    {
                        char c1 = (*c) & 0xFF,
                             c2 = (*c) >> 8;
                        std::cout << c1;
                        if(c2) std::cout << c2;
                        ++c;
                    }
                }
                break;
            case TRAP_HALT:
                {
                    std::cout << "HALT";
                    running = 0;
                }
                break;
        }
    }
}
#pragma GCC diagnostic pop

void (*op_table[16])(uint16_t) = {
    exec<0>, exec<1>, exec<2>, exec<3>,
    exec<4>, exec<5>, exec<6>, exec<7>,
    NULL, exec<9>, exec<10>, exec<11>,
    exec<12>, exec<13>, exec<14>, exec<15>
};
