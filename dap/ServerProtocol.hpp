#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include "JsonRPC.hpp"
#include "SocketBase.hpp"

namespace dap
{
class ServerProtocol
{
    JsonRPC m_rpc;

public:
    ServerProtocol();
    virtual ~ServerProtocol();

    void Initialize(SocketBase::Ptr_t conn);
};
};     // namespace dap
#endif // PROTOCOL_HPP
