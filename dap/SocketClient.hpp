#ifndef CLSOCKETCLIENT_H
#define CLSOCKETCLIENT_H

#include "Socket.hpp"
#include "dap_exports.hpp"

#include <wx/string.h>

using namespace std;
namespace dap
{
class WXDLLIMPEXP_DAP SocketClient : public Socket
{
    wxString m_path;

public:
    SocketClient();
    virtual ~SocketClient();

    /**
     * @brief connect to a remote server using ip/port
     * when using non-blocking mode, wouldBlock will be set to true
     * incase the connect fails
     */
    bool ConnectRemote(const wxString& address, int port);

    /**
     * @brief connect using connection wxString
     * @param connectionString in the format of tcp://127.0.0.1:1234
     * @return
     */
    bool Connect(const wxString& connectionString);
};
};     // namespace dap
#endif // CLSOCKETCLIENT_H
