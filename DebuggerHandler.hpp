#ifndef DEBUGGERHANDLER_HPP
#define DEBUGGERHANDLER_HPP

#include "dap/Exception.hpp"
#include "dap/Process.hpp"
#include "dap/dap.hpp"
#include <memory>
#include <vector>

class DebuggerHandler
{
public:
    typedef shared_ptr<DebuggerHandler> Ptr_t;

protected:
    dap::Process* m_process = nullptr;
    vector<dap::ProtocolMessage::Ptr_t> m_outgoingQueue;

public:
    DebuggerHandler();
    virtual ~DebuggerHandler();
    
    /**
     * @brief Get next message from the queue
     */
    dap::ProtocolMessage::Ptr_t TakeNextMessage();
    
    /**
     * @brief add message to the outgoing queue
     */
    void PushMessage(dap::ProtocolMessage::Ptr_t message);
    
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

    ///----------------------------------------------------------------------
    /// Any backend should override the below pure virtual methods
    ///----------------------------------------------------------------------
    /**
     * @brief process raw debugger output
     * @return message to be sent to the IDE (can be nullptr)
     */
    virtual void OnDebuggerStdout(const string& message) = 0;

    /**
     * @brief process raw debugger output
     * @return message to be sent to the IDE (can be nullptr)
     */
    virtual void OnDebuggerStderr(const string& message) = 0;

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

    /**
     * @brief handle configuration done from IDE -> debugger
     * @throws dap::Exception
     */
    virtual void OnConfigurationDoneRequest(dap::ProtocolMessage::Ptr_t message) = 0;

    /**
     * @brief fronend requested to set breakpoints
     * @throws dap::Exception
     */
    virtual void OnSetBreakpoints(dap::ProtocolMessage::Ptr_t message) = 0;

    /**
     * @brief Process 'Threads' request
     */
    virtual void OnThreads(dap::ProtocolMessage::Ptr_t message) = 0;
    
    ///----------------------------------------------------------------------
    /// Pure virtual methods end
    ///----------------------------------------------------------------------
};

#endif // DEBUGGERHANDLER_HPP
