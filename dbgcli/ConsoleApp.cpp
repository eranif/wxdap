#include "ConsoleApp.hpp"
#include "MainFrame.hpp"
#include "dap/Client.hpp"
#include "dap/Exception.hpp"
#include "dap/Log.hpp"
#include <wx/log.h>

IMPLEMENT_APP(DAPCli)

DAPCli::DAPCli()
    : wxApp()
{
}

DAPCli::~DAPCli() {}

void DAPCli::DoExitApp() {}

bool DAPCli::OnInit()
{
    SetAppName("DAPCli");
    wxLog::EnableLogging(false);

    m_parser.SetCmdLine(wxAppConsole::argc, wxAppConsole::argv);
    if(!DoParseCommandLine())
        return false;

    MainFrame* frame = new MainFrame(nullptr);
    frame->Show();
    SetTopWindow(frame);
    return true;
}

int DAPCli::OnExit() { return true; }
int DAPCli::OnRun()
{
    return wxApp::OnRun();
}

bool DAPCli::DoParseCommandLine() { return true; }
