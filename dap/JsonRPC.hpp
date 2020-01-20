#ifndef JSONRPC_HPP
#define JSONRPC_HPP

#include "SocketBase.hpp"
#include "dap.hpp"
#include <unordered_map>

namespace dap
{
class JsonRPC
{
protected:
    SocketBase::Ptr_t m_acceptSocket;
    SocketBase::Ptr_t m_client;
    string m_buffer;

protected:
    int ReadHeaders(unordered_map<string, string>& headers);

public:
    JsonRPC();
    ~JsonRPC();

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
