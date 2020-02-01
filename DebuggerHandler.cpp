#include "DebuggerHandler.hpp"

DebuggerHandler::DebuggerHandler() {}

DebuggerHandler::~DebuggerHandler() {}

pair<string, string> DebuggerHandler::Read() const
{
    if(!m_process || !m_process->IsAlive()) {
        throw dap::Exception("Debugger process is not running");
    }
    return m_process->Read();
}
