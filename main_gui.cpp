#include <iostream>
#include <memory>
#include <wx/wxprec.h>
#include <wx/wx.h>
#ifndef _WIN32
#include <signal.h>
#endif

#include "gui/FPTThread.hpp"
#include "gui/FPTFrame.hpp"

class FPTApp : public wxApp
{
    // std::unique_ptr<FPTFrame> frame;
public:
    virtual bool OnInit();
};

wxIMPLEMENT_APP(FPTApp);

bool FPTApp::OnInit()
{
    FPTFrame* frame = new FPTFrame();
    // frame = std::make_unique<FPTFrame>();
    frame->Show(true);
    return true;
}