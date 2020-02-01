#include "GdbHandler.hpp"
#include "utils.hpp"

GdbHandler::GdbHandler() {}

GdbHandler::~GdbHandler() { DELETE_PTR(m_process); }

void GdbHandler::OnLaunchRequest(dap::ProtocolMessage::Ptr_t message) {}

void GdbHandler::StartDebugger(const string& debuggerExecutable, const string& wd)
{
    m_process = dap::ExecuteProcess(debuggerExecutable + " -i=mi", wd);
    if(!m_process) {
        throw dap::Exception("Failed to start debugger process: " + debuggerExecutable);
    }
}
