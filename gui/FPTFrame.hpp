#pragma once

#include <vector>
#include <memory>
#include "../memory.hpp"
#include "FPTThread.hpp"

class FPTFrame : public wxFrame
{
    std::vector<std::shared_ptr<FPTThread>> thd_vec;
public:
    FPTFrame();
    
	wxString objectPath;
	wxTextCtrl* mTextBox;
	void OpenFile( wxCommandEvent& event );
};

enum
{
    ID_Load = 1,
    ID_Text,
};

FPTFrame::FPTFrame()
        : wxFrame(NULL, wxID_ANY, "FPT")
{
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_Load, "&Load...\tCtrl-L",
                     "Load an object file");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);
    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");
    
    Bind(wxEVT_MENU, 
        [=](wxCommandEvent&) {
            wxFileDialog *OpenDialog = new wxFileDialog(
                this, _("Choose a file to open"), wxEmptyString, wxEmptyString,
                _("Object files (*.obj)|*.obj|Any files (*.*)|*.*"),
                wxFD_OPEN, wxDefaultPosition);

            // Creates a "open file" dialog 
            if (OpenDialog->ShowModal() == wxID_OK) // if the user click "Open" instead of "cancel"
            {
                objectPath = OpenDialog->GetPath();
                
                thd_vec.push_back(std::make_shared<FPTThread>(mTextBox, objectPath)); // auto runs & deletes itself when finished
            }
        },
        ID_Load);
    Bind(wxEVT_MENU, 
        [=](wxCommandEvent&) { 
            wxMessageBox("This is a wxWidgets Hello World example", 
                        "About Hello World", wxOK | wxICON_INFORMATION); 
        }, 
        wxID_ABOUT);
    Bind(wxEVT_MENU,
        [=](wxCommandEvent&) {
            Close(true);
        }, 
        wxID_EXIT);

	mTextBox = new wxTextCtrl(
		this, ID_Text, _(""), wxDefaultPosition, wxDefaultSize, 
		wxTE_MULTILINE | wxTE_RICH | wxTE_READONLY , 
        wxDefaultValidator, wxTextCtrlNameStr);
    mTextBox->SetFont( wxFont(12, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
    mTextBox->Bind(wxEVT_CHAR, [=](wxKeyEvent &event) {
        input_buffer.push(event.GetUnicodeKey());
        event.Skip();
    });
}