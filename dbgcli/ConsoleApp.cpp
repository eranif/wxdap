#include "ConsoleApp.hpp"
#include "dap/Client.hpp"
#include "dap/Exception.hpp"
#include "dap/Log.hpp"
#include <wx/log.h>

IMPLEMENT_APP_CONSOLE(DAPCli)

DAPCli::DAPCli()
    : wxAppConsole()
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

    return true;
}

int DAPCli::OnExit() { return true; }
int DAPCli::OnRun()
{
    InitializeClient();
    return wxAppConsole::OnRun();
}
bool DAPCli::DoParseCommandLine() { return true; }

/// Initialize the DAP client:
/// - Bind events
/// - Connect
/// - And launch our debuggee process
void DAPCli::InitializeClient()
{
    dap::Log::OpenStdout(dap::Log::Dbg);
    LOG_INFO() << "Starting client...";

    m_client.Bind(wxEVT_DAP_STOPPED_EVENT, &DAPCli::OnStopped, this);
    m_client.Bind(wxEVT_DAP_INITIALIZED_EVENT, &DAPCli::OnInitialized, this);
    m_client.Bind(wxEVT_DAP_EXITED_EVENT, &DAPCli::OnExited, this);
    m_client.Bind(wxEVT_DAP_TERMINATED_EVENT, &DAPCli::OnTerminated, this);
    m_client.Bind(wxEVT_DAP_STACKTRACE_RESPONSE, &DAPCli::OnStackTrace, this);

    if(!m_client.Connect("tcp://127.0.0.1:12345", 10)) {
        LOG_ERROR() << "Error: failed to connect to server";
        exit(1);
    }
    LOG_INFO() << "Connected!" << endl;

    // This part is done in mode **sync**
    m_client.Initialize();
    m_client.ConfigurationDone();
    m_client.Launch({ R"(C:\Users\eran\Downloads\testclangd\Debug\testclangd.exe)" });
}

/// DAP server stopped. This can happen for multiple reasons:
/// - exception
/// - breakpoint hit
/// - step (user previously issued `Next` command)
void DAPCli::OnStopped(DAPEvent& event)
{
    // got stopped event
    dap::StoppedEvent* stopped_data = event.GetDapEvent()->As<dap::StoppedEvent>();
    if(stopped_data) {
        LOG_INFO() << "Stopped reason:" << stopped_data->reason << endl;
        LOG_INFO() << "All threads stopped:" << stopped_data->allThreadsStopped << endl;
        LOG_INFO() << "Stopped thread ID:" << stopped_data->threadId
                   << "(active thread ID:" << m_client.GetActiveThreadId() << ")" << endl;

        if(stopped_data->reason == "exception") {
            // apply breakpoints
            m_client.SetBreakpointsFile("main.cpp", { { 17, "" } });
            // Continue
            m_client.Continue();
        } else if(stopped_data->reason == "breakpoint" /* breakpoint hit */
                  || stopped_data->reason == "step" /* user called "Next" */) {
            // request the stack for the stopped thread
            m_client.GetFrames(m_client.GetActiveThreadId());
            m_client.Next(m_client.GetActiveThreadId());
        }
    }
}

/// Received a response to `GetFrames()` call
void DAPCli::OnStackTrace(DAPEvent& event)
{
    dap::StackTraceResponse* stack_trace_data = event.GetDapResponse()->As<dap::StackTraceResponse>();
    if(stack_trace_data) {
        LOG_INFO() << "Stack trace:" << endl;
        for(const auto& stack : stack_trace_data->stackFrames) {
            LOG_INFO() << stack.id << "," << stack.name << "," << stack.source.path << "," << stack.line << endl;
        }
    }
}
/// DAP server responded to our `initialize` request
void DAPCli::OnInitialized(DAPEvent& event)
{
    // got initialized event
    LOG_INFO() << "Received" << event.GetDapEvent()->event << "event" << endl;
}

/// Debuggee process exited, print the exit code
void DAPCli::OnExited(DAPEvent& event)
{
    LOG_INFO() << "Debuggee exited. Exit code:" << event.GetDapEvent()->As<dap::ExitedEvent>()->exitCode << endl;
}

/// Debug session terminated
void DAPCli::OnTerminated(DAPEvent& event)
{
    wxUnusedVar(event);
    LOG_INFO() << "Session terminated!" << endl;
}
