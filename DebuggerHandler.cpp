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

dap::ProtocolMessage::Ptr_t DebuggerHandler::TakeNextMessage()
{
    if(m_outgoingQueue.empty()) {
        return nullptr;
    }
    dap::ProtocolMessage::Ptr_t msg = m_outgoingQueue.front();
    m_outgoingQueue.erase(m_outgoingQueue.begin());
    return msg;
}

void DebuggerHandler::PushMessage(dap::ProtocolMessage::Ptr_t message)
{
    if(message) {
        m_outgoingQueue.push_back(message);
    }
}
