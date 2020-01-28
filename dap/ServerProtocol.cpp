#include "Log.hpp"
#include "ServerProtocol.hpp"

dap::ServerProtocol::ServerProtocol(SocketBase::Ptr_t conn)
    : m_conn(conn)
{
}

dap::ServerProtocol::~ServerProtocol() {}

void dap::ServerProtocol::Initialize()
{
    // Attempt to read something from the network
    enum eState { kWaitingInitRequest, kDone };
    eState state = kWaitingInitRequest;
    while(true) {
        string network_buffer;
        if(m_conn->Read(network_buffer, 10) == dap::SocketBase::kSuccess) {

            LOG_DEBUG1() << "Read: " << network_buffer;

            // Append the buffer to what we already have
            m_rpc.AppendBuffer(network_buffer);

            // Try to construct a message and process it
            dap::ProtocolMessage::Ptr_t request = m_rpc.ProcessBuffer();
            if(request) {
                switch(state) {
                case kWaitingInitRequest: {
                    if(request->type == "request" && request->As<dap::InitializeRequest>()) {
                        dap::InitializeResponse initResponse;
                        m_rpc.Send(initResponse, m_conn);
                        LOG_DEBUG() << "Sending InitializeRequest";

                        // Send InitializedEvent
                        dap::InitializedEvent initEvent;
                        m_rpc.Send(initEvent, m_conn);
                        LOG_DEBUG() << "Sending InitializedEvent";
                        LOG_INFO() << "Initialization completed";
                        state = kDone;
                    }
                    break;
                }
                case kDone: {
                    return;
                }
                }
            }
        }
    }
}

void dap::ServerProtocol::Check(function<void(dap::ProtocolMessage::Ptr_t)> onNetworkMessage)
{
    string content;
    if(m_conn->Read(content) == SocketBase::kSuccess) {
        m_rpc.AppendBuffer(content);
        dap::ProtocolMessage::Ptr_t message = m_rpc.ProcessBuffer();
        if(message) {
            // Call the handler
            onNetworkMessage(message);
        }
    }
}

void dap::ServerProtocol::ProcessGdbMessage(dap::ProtocolMessage::Ptr_t message)
{
    // message was generated by the GDB driver. Send it over to the client
}