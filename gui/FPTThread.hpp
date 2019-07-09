#pragma once

#include <memory>
#include <wx/wxprec.h>
#include <wx/wx.h>

class FPTThread : public wxThread
{
    bool mInit;
    wxTextCtrl* mBox;

public:
    FPTThread(wxTextCtrl* box, wxString objectPath);

protected:
    virtual ExitCode Entry();
};