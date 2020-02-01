#ifndef DEBUGGERHANDLER_HPP
#define DEBUGGERHANDLER_HPP

#include "dap/dap.hpp"
class DebuggerHandler
{
public:
    DebuggerHandler();
    virtual ~DebuggerHandler();
    
    /**
     * @brief handle launch request
     */
    virtual void OnLaunch(dap::ProtocolMessage::Ptr_t message) = 0;
};

#endif // DEBUGGERHANDLER_HPP
