#ifndef DEBUGGERHANDLER_HPP
#define DEBUGGERHANDLER_HPP

#include "dap/Exception.hpp"
#include "dap/Process.hpp"
#include "dap/dap.hpp"
#include <memory>

class DebuggerHandler
{
public:
    typedef shared_ptr<DebuggerHandler> Ptr_t;

protected:
    dap::Process* m_process = nullptr;

public:
    DebuggerHandler();
    virtual ~DebuggerHandler();
    
    /**
     * @brief is the debugger backend process alive?
     */
    bool IsAlive() const { return m_process && m_process->IsAlive(); }
    
    /**
     * @brief read from the process stdout/stderr
     * @return pair of strings: stdout/stderr
     * @throws dap::Exception
     */
    pair<string, string> Read() const;
    
    /**
     * @brief launch the debugger process
     * @param debuggerExecutable
     * @param wd working directory
     * @throws dap::Exception
     */
    virtual void StartDebugger(const string& debuggerExecutable, const string& wd) = 0;

    /**
     * @brief handle launch request set from the IDE -> debugger
     * @throws dap::Exception
     */
    virtual void OnLaunchRequest(dap::ProtocolMessage::Ptr_t message) = 0;
};

#endif // DEBUGGERHANDLER_HPP
