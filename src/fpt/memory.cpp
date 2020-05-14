#include <iostream>
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#elif _ANDROID
#include <unistd.h>
#include <termios.h>
#include <sys/mman.h>
#else
#include <sys/select.h>
#include <unistd.h>
#include <sys/termios.h>
#include <sys/mman.h>
#endif

#include "memory.hpp"
#include "InputBuffer.hpp"

uint16_t memory[UINT16_MAX];
InputBuffer input_buffer;

int check_key()
{
#ifdef _WIN32
    return kbhit();
#else
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
#endif
}

uint16_t mem_read(uint16_t address)
{
    if (address == MR_KBSR)
    {
        char c = input_buffer.pop();
        if(c) {
            memory[MR_KBSR] = 1 << 15;
            memory[MR_KBDR] = (uint16_t) c;
        } else {
            memory[MR_KBSR] = 0;
        }
    }
    return memory[address];
}
