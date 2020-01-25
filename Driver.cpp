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

void Driver::ProcessRequest(dap::Request::Ptr_t request)
{
    if(!m_initialized && (request->type == "initialize")) {
        // Send back "initialize response"
        m_initialized = true;
    } else if(m_initialized) {
        
    } else {
        // We can only accept initialize request at this stage
    }
}

dap::ProtocolMessage::Ptr_t Driver::Check()
{
    dap::ProtocolMessage::Ptr_t message = m_queue.pop(chrono::milliseconds(1));
    return message;
}
