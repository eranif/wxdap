#include "Driver.hpp"
#include "dap/Exception.hpp"
#include "dap/Log.hpp"
#include "dap/Process.hpp"
#include "dap/StringUtils.hpp"
#include "utils.hpp"
#include <sstream>
#include <thread>

Driver::Driver(const CommandLineParser& parser, DebuggerHandler::Ptr_t handler)
{
    m_backend = handler;
    if(!m_backend) {
        throw dap::Exception("Unable to construct Driver. You must provide a backend");
    }
    m_backend->StartDebugger(parser.GetDebuggerExec(), ".");
}

Driver::~Driver() {}

bool Driver::IsAlive() const { return m_backend->IsAlive(); }

void Driver::ProcessNetworkMessage(dap::ProtocolMessage::Ptr_t message)
{
    // Handle DAP request message
    LOG_DEBUG() << "<==" << message->To().ToString();
    if(message->type == "request") {
        dap::Request* request = message->As<dap::Request>();
        if(request->command == "launch") {
            OnLaunch(message);
        } else if(request->command == "setBreakpoints") {
            OnSetBreakpoints(message);
        } else if(request->command == "configurationDone") {
            OnCofigurationDone(message);
        } else if(request->command == "threads") {
            OnThreads(message);
        } else if(request->command == "scopes") {
            OnScopes(message);
        }
    }
}

void Driver::Check()
{
    auto output = m_backend->Read();
    if(!output.first.empty()) {
        LOG_DEBUG1() << "gdb (stdout):" << output.first;
        // Process ALL messages here
        m_backend->OnDebuggerStdout(output.first);
    }
    if(!output.second.empty()) {
        // Process the raw buffer
        LOG_DEBUG1() << "gdb (stderr):" << output.second;
        // Process ALL messages here
        m_backend->OnDebuggerStderr(output.first);
    }

    auto msg = m_backend->TakeNextMessage();
    while(msg) {
        m_onGdbOutput(msg);
        msg = m_backend->TakeNextMessage();
    }
}

void Driver::OnLaunch(dap::ProtocolMessage::Ptr_t request)
{
    try {
        m_backend->OnLaunchRequest(request);
    } catch(dap::Exception& e) {
        LOG_ERROR() << "Launch error:" << e.What();
        ReportError<dap::LaunchResponse>(request->seq, e.What());
    }
}

void Driver::OnSetBreakpoints(dap::ProtocolMessage::Ptr_t request)
{
    try {
        m_backend->OnSetBreakpoints(request);
    } catch(dap::Exception& e) {
        LOG_ERROR() << "SetBreakpoints error:" << e.What();
        ReportError<dap::SetBreakpointsResponse>(request->seq, e.What());
    }
}

void Driver::SetHandler(DebuggerHandler::Ptr_t handler) { m_backend = handler; }

void Driver::OnCofigurationDone(dap::ProtocolMessage::Ptr_t request)
{
    try {
        m_backend->OnConfigurationDoneRequest(request);
    } catch(dap::Exception& e) {
        LOG_ERROR() << "ConfigurationDone error:" << e.What();
        ReportError<dap::ConfigurationDoneResponse>(request->seq, e.What());
    }
}

void Driver::OnThreads(dap::ProtocolMessage::Ptr_t request)
{
    try {
        m_backend->OnThreads(request);
    } catch(dap::Exception& e) {
        LOG_ERROR() << "OnThreads error:" << e.What();
        ReportError<dap::ThreadsResponse>(request->seq, e.What());
    }
}

void Driver::OnScopes(dap::ProtocolMessage::Ptr_t request)
{
    try {
        m_backend->OnScopes(request);
    } catch(dap::Exception& e) {
        LOG_ERROR() << "OnScopes error:" << e.What();
        ReportError<dap::ScopesResponse>(request->seq, e.What());
    }
}
