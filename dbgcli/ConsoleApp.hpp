#ifndef CONSOLEAPP_HPP
#define CONSOLEAPP_HPP

#include "dap/Client.hpp"
#include "dap/DAPEvent.hpp"
#include <wx/app.h>
#include <wx/cmdline.h>

// -----------------------------------------------------------
// -----------------------------------------------------------
using namespace dap;

class DAPCli : public wxAppConsole
{
    wxCmdLineParser m_parser;
    dap::Client m_client;

protected:
    bool DoParseCommandLine();

    void OnStopped(DAPEvent& event);
    void OnStackTrace(DAPEvent& event);
    void OnInitialized(DAPEvent& event);
    void OnExited(DAPEvent& event);
    void OnTerminated(DAPEvent& event);
    void InitializeClient();

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
