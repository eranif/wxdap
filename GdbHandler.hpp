#ifndef GDBHANDLER_HPP
#define GDBHANDLER_HPP

#include "DebuggerHandler.hpp"

class GdbHandler : public DebuggerHandler
{
public:
    GdbHandler();
    virtual ~GdbHandler();

public:
    void OnLaunchRequest(dap::ProtocolMessage::Ptr_t message) override;
    void StartDebugger(const string& debuggerExecutable, const string& wd) override;
    
};

#endif // GDBHANDLER_HPP
