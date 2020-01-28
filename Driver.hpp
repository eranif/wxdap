#ifndef DRIVER_HPP
#define DRIVER_HPP

#include "CommandLineParser.hpp"
#include "dap/JsonRPC.hpp"
#include "dap/SocketBase.hpp"
#include "dap/dap.hpp"
#include <dap/Process.hpp>
#include <string>
#include <vector>

using namespace std;
/// Control interface to the underlying debugger
class Driver
{
    dap::Process* m_gdb = nullptr;
    dap::Queue<dap::ProtocolMessage::Ptr_t> m_queue;
    string m_stdout;
    string m_stderr;

protected:
    dap::ProtocolMessage::Ptr_t ProcessGdbStdout();
    dap::ProtocolMessage::Ptr_t ProcessGdbStderr();

public:
    Driver(const CommandLineParser& parser);
    ~Driver();

    bool IsAlive() const;

    /**
     * @brief check for incoming messages from gdb and process them
     */
    void Check(function<void(dap::ProtocolMessage::Ptr_t)> onGdbMessage);

    /**
     * @brief handle DAP message coming from the network
     */
    void ProcessNetworkMessage(dap::ProtocolMessage::Ptr_t message);
};

#endif // DRIVER_HPP
