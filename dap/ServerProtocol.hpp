#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include "JsonRPC.hpp"
#include "SocketBase.hpp"

namespace dap
{
class ServerProtocol
{
    JsonRPC m_rpc;
    SocketBase::Ptr_t m_conn;
    function<void(dap::ProtocolMessage::Ptr_t)> m_onNetworkMessage = nullptr;

public:
    ServerProtocol(SocketBase::Ptr_t conn);
    virtual ~ServerProtocol();

    void Initialize();

    /**
     * @brief register a callback for handling network messages
     */
    void RegisterNetworkCallback(function<void(dap::ProtocolMessage::Ptr_t)> onNetworkMessage)
    {
        m_onNetworkMessage = onNetworkMessage;
    }

    /**
     * @brief Check to see if any messages have arrived on the network
     * and process them
     */
    void Check();

    /**
     * @brief process gdb output
     */
    void ProcessGdbMessage(dap::ProtocolMessage::Ptr_t message);
};
};     // namespace dap
#endif // PROTOCOL_HPP
