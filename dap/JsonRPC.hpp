#ifndef JSONRPC_HPP
#define JSONRPC_HPP

#include "Queue.hpp"
#include "SocketBase.hpp"
#include "dap.hpp"
#include <atomic>
#include <thread>
#include <unordered_map>

namespace dap
{
struct Reader {
    thread* thr = nullptr;
    SocketBase* conn = nullptr;
    atomic_bool shutdown_flag;
    atomic_bool terminated_flag;
    Reader()
    {
        shutdown_flag.store(false);
        terminated_flag.store(false);
    }
    void Cleanup();
};

class JsonRPC
{
protected:
    SocketBase::Ptr_t m_acceptSocket;
    string m_buffer;
    Queue<string> m_incQueue;
    Reader m_reader;

protected:
    int ReadHeaders(unordered_map<string, string>& headers);

public:
    JsonRPC();
    ~JsonRPC();

    /**
     * @brief provide input buffer.
     * NOTE: this method is intended for testing purposes and should not be used
     */
    void SetBuffer(const string& buffer);

    /**
     * @brief start JSON RPC server on a connection string
     * @throws dap::SocketException
     */
    void ServerStart(const string& connectString);

    /**
     * @brief wait for new connection for 1 second
     * @throws dap::SocketException
     */
    bool WaitForNewConnection();

    /**
     * @brief attempt to constuct a message from the buffer read
     * If successful, return the message and consume the buffer
     */
    ProtocolMessage::Ptr_t ProcessBuffer();

    /**
     * @brief read data from the socket. Return true if something was read from the socket
     */
    bool Read();

    /**
     * @brief send message over the socket
     */
    void WriteMessge(ProtocolMessage::Ptr_t message);
};
};     // namespace dap
#endif // JSONRPC_HPP
