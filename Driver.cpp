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
    LOG_DEBUG() << "<==" << message->To().Format();
    if(message->type == "request" && message->As<dap::Request>()->command == "launch") {
        // Handle launch command
        OnLaunch(message);
    }
}

void Driver::Check()
{
    auto output = m_backend->Read();
    if(!output.first.empty()) {
        LOG_DEBUG1() << "gdb (stdout):" << output.first;
        // Process ALL messages here
        auto msg = m_backend->OnDebuggerStdout(output.first);
        while(msg) {
            m_onGdbOutput(msg);
            msg = m_backend->OnDebuggerStdout("");
        }
    }
    if(!output.second.empty()) {
        // Process the raw buffer
        LOG_DEBUG1() << "gdb (stderr):" << output.second;
        // Process ALL messages here
        auto msg = m_backend->OnDebuggerStderr(output.first);
        while(msg) {
            m_onGdbOutput(msg);
            msg = m_backend->OnDebuggerStderr("");
        }
    }
}

void Driver::OnLaunch(dap::ProtocolMessage::Ptr_t request)
{
    try {
        m_backend->OnLaunchRequest(request);
    } catch(dap::Exception& e) {
        LOG_ERROR() << "Launch error:" << e.What();
        ReportLaunchError(request->seq, e.What());
    }
}

void Driver::ReportLaunchError(int seq, const string& what)
{
    dap::LaunchResponse* response = new dap::LaunchResponse();
    response->message = what;
    response->request_seq = seq;
    response->success = false;
    if(m_onGdbOutput) {
        m_onGdbOutput(dap::ProtocolMessage::Ptr_t(response));
    }
}

void Driver::SetHandler(DebuggerHandler::Ptr_t handler) { m_backend = handler; }
