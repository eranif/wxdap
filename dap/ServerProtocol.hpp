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

public:
    ServerProtocol(SocketBase::Ptr_t conn);
    virtual ~ServerProtocol();

    void Initialize();

    /**
     * @brief Check to see if any messages have arrived on the network
     * and process them
     */
    void Check(function<void(dap::ProtocolMessage::Ptr_t)> onNetworkMessage);

    /**
     * @brief process gdb output
     */
    void ProcessGdbMessage(dap::ProtocolMessage::Ptr_t message);
};
};     // namespace dap
#endif // PROTOCOL_HPP
