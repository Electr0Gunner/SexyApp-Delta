#include <wx/wx.h>
#include "SexyTools.h"

using namespace Sexy;

int main(int argc, char** argv)
{
    wxDISABLE_DEBUG_SUPPORT();
    new ToolsApp;
    return wxEntry(argc, argv);
}