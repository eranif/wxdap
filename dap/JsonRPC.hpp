#ifndef JSONRPC_HPP
#define JSONRPC_HPP

#include "Exception.hpp"
#include "Queue.hpp"
#include "dap.hpp"
#include "dap_exports.hpp"
#include <atomic>
#include <string>
#include <thread>
#include <unordered_map>
#include <wx/object.h>

namespace dap
{
class WXDLLIMPEXP_DAP JsonRPC
{
protected:
    wxString m_buffer;

protected:
    int ReadHeaders(std::unordered_map<wxString, wxString>& headers);
    Json DoProcessBuffer();

public:
    JsonRPC();
    ~JsonRPC();

    /**
     * @brief provide input buffer.
     * NOTE: this method is intended for testing purposes and should not be used
     */
    void SetBuffer(const wxString& buffer);

    /**
     * @brief append content to the existing buffer
     */
    void AppendBuffer(const wxString& buffer);

    /**
     * @brief Check if we have a complete Json message in the internal buffer and invoke callback
     * If successful, callback is called. Note that it will get called as long there are complete messages in the
     * internal buffer
     * @param callback
     * @param o user object that is sent back to the callback
     */
    void ProcessBuffer(std::function<void(const Json&, wxObject*)> callback, wxObject* o);

    /**
     * @brief send protocol message over the network
     * TransportPtr must have a Send(const wxString&) method
     */
    template <typename TransportPtr>
    void Send(ProtocolMessage& msg, TransportPtr conn) const
    {
        if(!conn) {
            throw Exception("Invalid connection");
        }
        wxString network_buffer;
        wxString payload = msg.ToString();
        network_buffer = "Content-Length: ";
        network_buffer += std::to_string(payload.length());
        network_buffer += "\r\n\r\n";
        network_buffer += payload;
        conn->Send(network_buffer);
    }

    /**
     * @brief send protocol message over the network
     * TransportPtr must have a Send(const wxString&) method
     */
    template <typename TransportPtr>
    void Send(ProtocolMessage::Ptr_t msg, TransportPtr conn) const
    {
        if(!msg) {
            throw Exception("Unable to send empty message");
        }
        if(!conn) {
            throw Exception("Invalid connection");
        }
        Send(*msg.get(), conn);
    }
};
};     // namespace dap
#endif // JSONRPC_HPP
