#ifndef DRIVER_HPP
#define DRIVER_HPP

#include "CommandLineParser.hpp"
#include "DebuggerHandler.hpp"
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
    dap::Queue<dap::ProtocolMessage::Ptr_t> m_queue;
    string m_stdout;
    string m_stderr;
    function<void(dap::ProtocolMessage::Ptr_t)> m_onGdbOutput = nullptr;
    DebuggerHandler::Ptr_t m_backend;
    friend class DebuggerHandler;

protected:
    void OnLaunch(dap::ProtocolMessage::Ptr_t request);
    void ReportLaunchError(int seq, const string& what);

public:
    Driver(const CommandLineParser& parser, DebuggerHandler::Ptr_t handler);
    ~Driver();

    /**
     * @brief set debugger backend
     */
    void SetHandler(DebuggerHandler::Ptr_t handler);

    /**
     * @brief register a callback for gdb output
     */
    void RegisterGdbOutputCallback(function<void(dap::ProtocolMessage::Ptr_t)> onGdbOutput)
    {
        m_onGdbOutput = onGdbOutput;
    }

    /**
     * @brief is gdb alive?
     */
    bool IsAlive() const;

    /**
     * @brief check for incoming messages from gdb and process them
     */
    void Check();

    /**
     * @brief handle DAP message coming from the network
     */
    void ProcessNetworkMessage(dap::ProtocolMessage::Ptr_t message);
};

#endif // DRIVER_HPP
