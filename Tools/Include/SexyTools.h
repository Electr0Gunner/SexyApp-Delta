#pragma once

#include <wx/wxprec.h>
 
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

namespace Sexy
{
    class ToolsApp : public wxApp
    {
        public:
            ToolsApp();
            ~ToolsApp();

            virtual bool OnInit();
    };

    class ToolsFrame : public wxFrame
    {
        enum
        {
            ID_TOOL_RESOURCE,
            ID_TOOL_PAK,
        };
        public:
            ToolsFrame();
        private:
            void DoPackaging(wxCommandEvent& event);

    };

} // namespace Sexy
