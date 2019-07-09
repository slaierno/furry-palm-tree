#include "FPTThread.hpp"
#include <wx/thread.h>
#include <wx/event.h>
#include "../lc3-hw.hpp"
#include "../memory.hpp"

FPTThread::FPTThread(wxTextCtrl* box, wxString objectPath) : wxThread(wxTHREAD_DETACHED), mBox(box) {
    if(!read_image(objectPath.c_str())) {
        std::cerr << "failed to load image: " << objectPath.c_str() << std::endl;
    } else {
        mInit = true;
        reg[R_PC] = PC_START;
        if (wxTHREAD_NO_ERROR == Create())
        {
            Run();
        }
    }
}

void* FPTThread::Entry() {
    if(!mInit) {
        std::cerr << "Object mObjectPath not loaded";
        return static_cast<ExitCode>(NULL);
    }
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();
    // do something here that takes a long time
    // it's a good idea to periodically check TestDestroy()
    while (!TestDestroy() && running)
    {
            wxStreamToTextRedirector redirect(mBox);
            /* INTERRUPT */
            //TODO
            
            /* FETCH */
            uint16_t instr = mem_read(reg[R_PC]++);
            uint16_t op = instr >> 12;

            op_table[op](instr);
    }
    restore_input_buffering();
    return static_cast<ExitCode>(NULL);
}