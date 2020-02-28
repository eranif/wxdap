#ifndef JSONRPC_HPP
#define JSONRPC_HPP

#include "Queue.hpp"
#include "Socket.hpp"
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
    JSON DoProcessBuffer();

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
     * @brief Check if we have a complete JSON message in the internal buffer and invoke callback
     * If successful, callback is called. Note that it will get called as long there are complete messages in the
     * internal buffer
     */
    void ProcessBuffer(function<void(const JSON& obj)> callback);

    /**
     * @brief send protocol message over the network
     */
    void Send(ProtocolMessage& msg, Socket::Ptr_t conn) const;
    /**
     * @brief send protocol message over the network
     */
    void Send(ProtocolMessage::Ptr_t msg, Socket::Ptr_t conn) const;
};
};     // namespace dap
#endif // JSONRPC_HPP
