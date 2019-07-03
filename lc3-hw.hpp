#pragma once

#include <iostream>
#include <stdint.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/types.h>
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#elif _ANDROID
    #include <unistd.h>
    #include <termios.h>
    #include <sys/mman.h>
#else
    #include <unistd.h>
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

/* Where the first program instruction is loaded */
/* 0x3000 is the default */
extern uint16_t PC_START;

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
    OP_XOR,    /* bitwise xor */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};

/* LC-3b extension where RES opcode is used for bit shift */
#define OP_SHF OP_RES
/* LC-3b extension where NOT is a special case of XOR */
#define OP_NOT OP_XOR

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
#else
void disable_input_buffering();
void restore_input_buffering();
#endif

void handle_interrupt(int signal);

extern int running;

extern void (*op_table[16])(uint16_t);
