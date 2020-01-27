#include "Log.hpp"
#include "ServerProtocol.hpp"

dap::ServerProtocol::ServerProtocol() {}

dap::ServerProtocol::~ServerProtocol() {}

void dap::ServerProtocol::Initialize(SocketBase::Ptr_t conn)
{
    // Attempt to read something from the network
    enum eState { kWaitingInitRequest, kDone };
    eState state = kWaitingInitRequest;
    while(true) {
        string network_buffer;
        if(conn->Read(network_buffer, 10) == dap::SocketBase::kSuccess) {

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
                        m_rpc.Send(initResponse, conn);
                        LOG_DEBUG() << "Sending InitializeRequest";

                        // Send InitializedEvent
                        dap::InitializedEvent initEvent;
                        m_rpc.Send(initEvent, conn);
                        LOG_DEBUG() << "Sending InitializedEvent";
                        LOG_INFO() << "Initialization completed";
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
