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
class JsonRPC
{
protected:
    string m_buffer;

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
     * @brief append content to the existing buffer
     */
    void AppendBuffer(const string& buffer);

    /**
     * @brief attempt to constuct a message from the buffer read
     * If successful, return the message and consume the buffer
     */
    ProtocolMessage::Ptr_t ProcessBuffer();

    /**
     * @brief send protocol message over the network
     */
    void Send(ProtocolMessage& msg, SocketBase::Ptr_t conn) const;
    /**
     * @brief send protocol message over the network
     */
    void Send(ProtocolMessage::Ptr_t msg, SocketBase::Ptr_t conn) const;
};
};     // namespace dap
#endif // JSONRPC_HPP
