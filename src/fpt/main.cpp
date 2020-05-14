#include <iostream>
#ifndef _WIN32
#include <signal.h>
#endif

#include <thread>
#include "lc3-hw.hpp"
#include "memory.hpp"
#include "lc3-debug.hpp"

int main(int argc, const char* argv[])
{
    bool debug_print = false;

    if (argc < 2)
    {
        /* show usage string */
        std::cout << "lc3 [image-file1] ..." << std::endl;
        exit(2);
    }

    for (int j = 1; j < argc; ++j)
    {
        if (j == 1 && (std::string("-g").compare(argv[j]) == 0)) {
            debug_print = true;
            continue;
        }
        if (!read_image(argv[j]))
        {
            std::cerr << "failed to load image: " << argv[j] << std::endl;
            exit(1);
        }
    }

    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    /* set the PC to starting position */
    reg[R_PC] = PC_START;

    std::thread kb_poll([=](){
        while(true) if(check_key())
            input_buffer.push(std::cin.get());
    });
    while (running)
    {
        /* INTERRUPT */
        //TODO
        
        /* FETCH */
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;

        if(debug_print) {
            std::cout << mcodeToString(instr) << std::endl;
        }
        op_table[op](instr);
    }
    kb_poll.join();
    
    restore_input_buffering();
}