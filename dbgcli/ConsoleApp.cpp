#include "ConsoleApp.hpp"
#include "dap/Client.hpp"
#include "dap/Exception.hpp"
#include "dap/Log.hpp"
#include <wx/log.h>

IMPLEMENT_APP_CONSOLE(DAPCli)

DAPCli::DAPCli() {}

DAPCli::~DAPCli() {}

void DAPCli::DoExitApp() {}

bool DAPCli::OnInit()
{
    SetAppName("DAPCli");
    wxLog::EnableLogging(false);

    m_parser.SetCmdLine(wxAppConsole::argc, wxAppConsole::argv);
    if(!DoParseCommandLine())
        return false;

    try {
        dap::Log::OpenStdout(dap::Log::Dbg);
        LOG_INFO() << "Starting client...";
        dap::Client client;
        if(!client.Connect("tcp://127.0.0.1:12345", 10)) {
            LOG_ERROR() << "Error: failed to connect to server";
            exit(1);
        }
        LOG_INFO() << "Connected!";

        // This part is done in mode **sync**
        client.Initialize();
        client.SetBreakpointsFile("main.cpp", { { 17, "" }, { 18, "" } });
        client.ConfigurationDone();
        client.Launch({ R"(C:\Users\eran\Downloads\testclangd\Debug\testclangd.exe)" });

        // Now that the initialization is completed, run the main loop
        while(client.IsConnected()) {
            // our callback we get keep called as long there are messages to process
            client.Check([&](JSON json) {
                auto msg = dap::ObjGenerator::Get().FromJSON(json);
                if(msg) {
                    LOG_DEBUG() << "<== " << msg->ToString();
                    // Stopped cause of breakpoint
                    if(msg->AsEvent() && msg->AsEvent()->event == "stopped") {
                        LOG_INFO() << "Stopped." << msg->As<dap::StoppedEvent>()->text;
                        // Ask for call stack
                        client.GetThreads();
                        LOG_INFO() << "Fetching threads...";
                    } else if(msg->AsResponse() && msg->AsResponse()->command == "threads") {
                        LOG_INFO() << "Fetching scopes...";
                        // Return variables for stack frame 0
                        client.GetScopes(0);
                    }
                }
            });
            this_thread::sleep_for(chrono::milliseconds(1));
        }

    } catch(dap::Exception& e) {
    }
    return true;
}

int DAPCli::OnExit() { return true; }

bool DAPCli::DoParseCommandLine() { return true; }
