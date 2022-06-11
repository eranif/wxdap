#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include "Socket.hpp"
#include "dap_exports.hpp"

#include <wx/string.h>

using namespace std;
namespace dap
{
class WXDLLIMPEXP_DAP SocketServer : public Socket
{
public:
    SocketServer();
    virtual ~SocketServer();

protected:
    /**
     * @throw clSocketException
     * @return port number
     */
    int CreateServer(const wxString& address, int port);

public:
    /**
     * @brief Create server using connection string
     * @return port number on success
     * @throw clSocketException
     */
    int Start(const wxString& connectionString);
    Socket::Ptr_t WaitForNewConnection(long timeout);
    /**
     * @brief same as above, however, return a pointer to the connection that should be freed by the caller
     */
    Socket* WaitForNewConnectionRaw(long timeout);
};
};     // namespace dap
#endif // CLSOCKETSERVER_H
