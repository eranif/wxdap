#include "ConsoleApp.hpp"

#include "MainFrame.hpp"
#include "dap/Client.hpp"
#include "dap/Exception.hpp"
#include "dap/Log.hpp"

#include <wx/cmdline.h>
#include <wx/filename.h>
#include <wx/log.h>

IMPLEMENT_APP(DAPCli)

DAPCli::DAPCli()
    : wxApp()
{
    m_ExecutableFileName = wxEmptyString;
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

    MainFrame* frame = new MainFrame(nullptr, m_ExecutableFileName);
    frame->Show();
    SetTopWindow(frame);
    return true;
}

int DAPCli::OnExit() { return true; }
int DAPCli::OnRun() { return wxApp::OnRun(); }

bool DAPCli::DoParseCommandLine()
{
    const wxCmdLineEntryDesc cmdLineDesc[] = {
        { wxCMD_LINE_SWITCH, "h", "help", "show this help message", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
        { wxCMD_LINE_SWITCH, "?", "?", "show this help message", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
        { wxCMD_LINE_PARAM, "", "", "Executable filename to debug", wxCMD_LINE_VAL_STRING,
          wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },
        { wxCMD_LINE_NONE }
    };

    m_parser.SetDesc(cmdLineDesc);

    switch(m_parser.Parse()) {
    case -1:
        return false; // The parameter -h was passed, help was given, so abort the app
    case 0:
        break; // OK, so break to deal with any parameters etc
    default:
        return false; // Some syntax error occurred. Abort
    }

    int paramCount = m_parser.GetParamCount();

    if(paramCount == 1) {
        const wxString& strParam = m_parser.GetParam(0);
        wxFileName fn(strParam);
        // Really important so that two same files with different names are not loaded
        // twice. Use the CurrentWorkingDirectory of the client instance to restore the
        // absolute path to the file.
        fn.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG |
                     wxPATH_NORM_SHORTCUT);
        const wxString& paramFullPath = fn.GetFullPath();

        if(wxFileName::FileExists(paramFullPath)) {
            m_ExecutableFileName = paramFullPath;
        }
    }

    return true;
}
