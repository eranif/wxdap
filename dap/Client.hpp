#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "JsonRPC.hpp"
#include "SocketBase.hpp"

namespace dap
{
class Client
{
    SocketBase::Ptr_t m_socket = nullptr;
    dap::JsonRPC m_rpc;
    Queue<string> m_inputQueue;
    atomic_bool m_shutdown;
    atomic_bool m_terminated;
    thread* m_readerThread = nullptr;
    size_t m_requestSeuqnce = 0;

protected:
    size_t GetNextSequence()
    {
        m_requestSeuqnce++;
        return m_requestSeuqnce;
    }

public:
    Client();
    virtual ~Client();
    
    /**
     * @brief Check if a new message arrived from the debugger server
     */
    void Check(function<void(JSON)> callback);
    
    /**
     * @brief Wait until connection is established
     */
    bool Connect(int timeoutSeconds);

    /**
     * @brief initiate the handshake between the server and the client
     */
    void Initialize();

    /**
     * @brief are we still connected?
     */
    bool IsConnected() const;

    /**
     * @brief set multiple breakpoints in a source file
     */
    void SetBreakpointsFile(const string& file, const vector<dap::SourceBreakpoint>& lines);
    
    /**
     * @brief tell the debugger that we are done and ready to start the main loop
     */
    void ConfigurationDone();
    /**
     * @brief start the debuggee
     */
    void Launch(const vector<string>& cmd);
};

};     // namespace dap
#endif // CLIENT_HPP
