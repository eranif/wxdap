#ifndef CONSOLEAPP_HPP
#define CONSOLEAPP_HPP

#include <wx/app.h>
#include <wx/cmdline.h>

// -----------------------------------------------------------
// -----------------------------------------------------------
class DAPCli : public wxAppConsole
{
    wxCmdLineParser m_parser;

protected:
    bool DoParseCommandLine();

public:
    DAPCli();
    virtual ~DAPCli();

    void DoExitApp();
    virtual bool OnInit();
    virtual int OnExit();
};

DECLARE_APP(DAPCli)

#endif // CONSOLEAPP_HPP
