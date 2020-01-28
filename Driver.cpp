#include "Driver.hpp"
#include "Exception.hpp"
#include "dap/Process.hpp"
#include "dap/StringUtils.hpp"
#include "utils.hpp"
#include <sstream>
#include <thread>

Driver::Driver(const CommandLineParser& parser)
{
    stringstream ss;
    ss << parser.GetGdb() << " -i=mi";
    m_gdb = dap::ExecuteProcess(ss.str());
}

Driver::~Driver() { DELETE_PTR(m_gdb); }

bool Driver::IsAlive() const { return m_gdb && m_gdb->IsAlive(); }

void Driver::ProcessNetworkMessage(dap::ProtocolMessage::Ptr_t message)
{
    // Handle DAP request message
}

void Driver::Check(function<void(dap::ProtocolMessage::Ptr_t)> onGdbMessage)
{
    auto output = m_gdb->Read();
    if(!output.first.empty()) {
        // Process the raw buffer
        m_stdout.append(output.first);
        auto msg = ProcessGdbStdout();
        if(msg) {
            onGdbMessage(msg);
        }
    }
    if(!output.second.empty()) {
        // Process the raw buffer
        m_stderr.append(output.second);
        auto msg = ProcessGdbStderr();
        if(msg) {
            onGdbMessage(msg);
        }
    }
}

dap::ProtocolMessage::Ptr_t Driver::ProcessGdbStdout()
{
    // Given a raw gdb output, convert it into
    return nullptr;
}

dap::ProtocolMessage::Ptr_t Driver::ProcessGdbStderr()
{
    // Given a raw gdb output, convert it into
    return nullptr;
}
