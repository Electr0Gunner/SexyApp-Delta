#include "SexyTools.h"
#include <wx/notebook.h>

// ----- Tool Includes -----

#include "PakTool.h"

using namespace Sexy;

// ----- ToolsApp -----

ToolsApp::ToolsApp()
{
}
    
ToolsApp::~ToolsApp()
{
}
    
bool ToolsApp::OnInit()
{
    ToolsFrame* frame = new ToolsFrame();
    frame->Show(true);
    return true;
}

// ----- ToolsFrame -----

ToolsFrame::ToolsFrame() : wxFrame(nullptr, wxID_ANY, "SexyTools", wxDefaultPosition, wxSize(500, 300))
{
    wxNotebook* aNotebook = new wxNotebook(this, wxID_ANY);

    wxPanel* aResourcePanel = new wxPanel(aNotebook, wxID_ANY);
    wxPanel* aPakToolPanel = new wxPanel(aNotebook, wxID_ANY);

    new wxStaticText(aResourcePanel, wxID_ANY, "Resource Generator goes here", wxPoint(20,20));
    new wxStaticText(aPakToolPanel, wxID_ANY, "Packing", wxPoint(10,10));
    new wxStaticText(aPakToolPanel, wxID_ANY, "Unpacking", wxPoint(10,80));

    aNotebook->AddPage(aResourcePanel, "Resource Gen");
    aNotebook->AddPage(aPakToolPanel, "PakTool");

    wxButton* aPackButton = new wxButton(aPakToolPanel, wxID_ANY, "Package Folder", wxPoint(10, 30));
    aPackButton->Bind(wxEVT_BUTTON, &ToolsFrame::DoPackaging, this);
    
    wxBoxSizer* aSizer = new wxBoxSizer(wxVERTICAL);
    aSizer->Add(aNotebook, 1, wxEXPAND);
    SetSizer(aSizer);
}

void ToolsFrame::DoPackaging(wxCommandEvent& event)
{
    PakTool::Pack("eh", "Later");
}