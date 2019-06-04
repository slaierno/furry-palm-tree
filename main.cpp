#include <iostream>

#include "lc3-hw.hpp"
#include "memory.hpp"

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        /* show usage string */
        std::cout << "lc3 [image-file1] ..." << std::endl;
        exit(2);
    }

    for (int j = 1; j < argc; ++j)
    {
        if (!read_image(argv[j]))
        {
            std::cerr << "failed to load image: " << argv[j] << std::endl;
            exit(1);
        }
    }

    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    /* set the PC to starting position */
    /* 0x3000 is the default */
    enum { PC_START = 0x3000 };
    reg[R_PC] = PC_START;

    while (running)
    {
        /* INTERRUPT */
        //TODO
        
        /* FETCH */
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;

        op_table[op](instr);
    }
    
    restore_input_buffering();
}