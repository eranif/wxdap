#ifndef CONSOLEAPP_HPP
#define CONSOLEAPP_HPP

#include <wx/app.h>
#include <wx/cmdline.h>

// -----------------------------------------------------------
// -----------------------------------------------------------
class DAPCli : public wxApp
{
    wxCmdLineParser m_parser;
    wxString m_ExecutableFileName;

protected:
    bool DoParseCommandLine();

public:
    DAPCli();
    virtual ~DAPCli();

    void DoExitApp();
    bool OnInit() override;
    int OnExit() override;
    int OnRun() override;
};

DECLARE_APP(DAPCli)

#endif // CONSOLEAPP_HPP
