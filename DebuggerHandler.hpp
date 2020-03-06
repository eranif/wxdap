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
    size_t m_sequence = 0;

    /**
     * @brief create new response of type T
     * @return
     */
    template <typename T>
    T* NewResponse(int requestSequence, bool success = true)
    {
        T* response = new T();
        response->seq = ++m_sequence;
        response->request_seq = requestSequence;
        response->success = success;
        return response;
    }

    /**
     * @brief create new event of type T
     * @return
     */
    template <typename T>
    T* NewEvent()
    {
        T* event = new T();
        event->seq = ++m_sequence;
        return event;
    }

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
     * @brief handle launch request sent from the IDE -> debugger
     * @throws dap::Exception
     */
    virtual void OnLaunchRequest(dap::ProtocolMessage::Ptr_t message) = 0;

    /**
     * @brief handle configuration done from IDE -> debugger
     * @throws dap::Exception
     */
    virtual void OnConfigurationDoneRequest(dap::ProtocolMessage::Ptr_t message) = 0;

    /**
     * @brief frontend requested to set breakpoints sent from the IDE -> debugger
     * @throws dap::Exception
     */
    virtual void OnSetBreakpoints(dap::ProtocolMessage::Ptr_t message) = 0;

    /**
     * @brief Process 'Threads' request sent from the IDE -> debugger
     */
    virtual void OnThreads(dap::ProtocolMessage::Ptr_t message) = 0;

    /**
     * @brief Process 'Scopes' request sent from the IDE -> debugger
     */
    virtual void OnScopes(dap::ProtocolMessage::Ptr_t message) = 0;

    /**
     * @brief Process 'stackTrace' request sent from the IDE -> debugger
     */
    virtual void OnStackTrace(dap::ProtocolMessage::Ptr_t message) = 0;
    /**
     * @brief process 'Variables' request, IDE->Debugger
     */
    virtual void OnVariables(dap::ProtocolMessage::Ptr_t message) = 0;
    
    ///----------------------------------------------------------------------
    /// Pure virtual methods end
    ///----------------------------------------------------------------------
};

#endif // DEBUGGERHANDLER_HPP
