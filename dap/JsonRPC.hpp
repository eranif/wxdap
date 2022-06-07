#ifndef JSONRPC_HPP
#define JSONRPC_HPP

#include "Queue.hpp"
#include "Socket.hpp"
#include "dap.hpp"
#include "dap_exports.hpp"
#include <atomic>
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
    JSON DoProcessBuffer();

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
     * @brief Check if we have a complete JSON message in the internal buffer and invoke callback
     * If successful, callback is called. Note that it will get called as long there are complete messages in the
     * internal buffer
     * @param callback
     * @param o user object that is sent back to the callback
     */
    void ProcessBuffer(std::function<void(const JSON&, wxObject*)> callback, wxObject* o);

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
