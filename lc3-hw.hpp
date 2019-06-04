#pragma once

#include <iostream>
#include <stdint.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/types.h>
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#elif
    #include <sys/termios.h>
    #include <sys/mman.h>
#endif

#include "memory.hpp"

enum {
    TRAP_GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
    TRAP_OUT = 0x21,   /* output a character */
    TRAP_PUTS = 0x22,  /* output a word string */
    TRAP_IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24, /* output a byte string */
    TRAP_HALT = 0x25   /* halt the program */
};

/* REGISTERS 
 * 
 * LC-3 has 10 total registers, each of which is 16 bits
 * * 8 general purpose registers (R0-R7)
 * * 1 program counter register (PC)
 * * 1 confition flags register (COND)
 */
enum {
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,
    R_COND,
    R_PSR,
    R_COUNT,
};
extern uint16_t reg[R_COUNT];

/* Instruction set
 * 
 * There are 16 opcodes in LC-3.
 * Each instruction is 16 bits long.
 * The 4 MSBs are the opcode, the rest are used to store parameters
 */
enum {
    OP_BR = 0, /* branch */
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};

/* LC-3b extension where RES opcode is used for bit shift */
#define OP_SHF OP_RES

/*
 * LSH DR, SR:
 * * ADD DR, SR, SR
 * 
 * NEG DR, SR1
 * * NOT DR, SR1
 * * ADD DR, DR, #1
 * 
 * SUB DR, SR1, SR2:
 * * NOT DR, SR2
 * * ADD DR, DR, #1
 * * ADD DR, DR, SR1
 * ----
 * * NEG DR, SR2
 * * ADD DR, SR1, DR
 * 
 * RSH DR, SR: ;REQUIRES R2 and R3 as temp registers
 * * AND R2, R2, #0
 * * ADD R2, R2, #1
 * * LOOP
 * * ADD R3, R2, #0 ; R3: output mask
 * * ADD R2, R2, R2 ; R2: input mask
 * * BRz END
 * * AND R4, SR, R2 ; R4: temp register
 * * BRz LOOP
 * * ADD DR, DR, R3
 * * BR LOOP
 * * END
 * 
 * XOR DR, SR1, SR2 ;Using A^B=(A+B)-2*(A&B)
 * * AND DR, SR1, SR2     ;DR :=                      (SR1 & SR2)
 * * NOT DR, DR           ;DR :=~DR                  ~(SR1 & SR2)
 * * ADD DR, DR, #1       ;DR :=-DR       =          -(SR1 & SR2)
 * * ADD DR, DR, DR       ;DR := DR + DR  =       - 2*(SR1 & SR2)
 * * ADD DR, DR, SR1      ;DR := DR + SR1 = A     - 2*(SR1 & SR2)
 * * ADD DR, DR, SR2      ;DR := DR + SR2 = A + B - 2*(SR1 & SR2)
 * ----
 * * AND DR, SR1, SR2
 * * NEG DR, DR
 * * ADD DR, DR
 * * ADD DR, DR, SR1
 * * ADD DR, DR, SR2
 */

/* CONDITION FLAGS */
enum {
    FL_POS = 0b001, /* P */
    FL_ZRO = 0b010, /* Z */
    FL_NEG = 0b100, /* N */
};

inline uint16_t sign_extend(uint16_t x, int bit_count) { if (x >> (bit_count - 1) & 1) x |= (0xFFFF << bit_count); return x; }
inline uint16_t swap16(uint16_t x) { return (x << 8) | (x >> 8); }

void update_flags(uint16_t r);
void read_image_file(FILE* file);
int read_image(const char* image_path);

#ifdef _WIN32
void ErrorExit (char* lpszMessage);
void disable_input_buffering();
void restore_input_buffering();
#elif
void disable_input_buffering();
void restore_input_buffering();
#endif

void handle_interrupt(int signal);

extern int running;

extern void (*op_table[16])(uint16_t);