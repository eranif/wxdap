#ifndef CONSOLEAPP_HPP
#define CONSOLEAPP_HPP

#include "dap/Client.hpp"
#include "dap/DAPEvent.hpp"
#include <wx/app.h>
#include <wx/cmdline.h>

// -----------------------------------------------------------
// -----------------------------------------------------------
using namespace dap;

class DAPCli : public wxApp
{
    wxCmdLineParser m_parser;

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
