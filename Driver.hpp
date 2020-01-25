#ifndef DRIVER_HPP
#define DRIVER_HPP

#include "CommandLineParser.hpp"
#include "dap/dap.hpp"
#include <dap/Process.hpp>
#include <string>
#include <vector>

using namespace std;
/// Control interface to the underlying debugger
class Driver
{
    dap::Process* m_gdb = nullptr;
    bool m_initialized = false;
    dap::Queue<dap::ProtocolMessage::Ptr_t> m_queue;

public:
    Driver(const CommandLineParser& parser);
    ~Driver();

    bool IsAlive() const;
    /**
     * @brief process DAP request coming from the client
     */
    void ProcessRequest(dap::Request::Ptr_t request);
    
    dap::ProtocolMessage::Ptr_t Check();
};

#endif // DRIVER_HPP
