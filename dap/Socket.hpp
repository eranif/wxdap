#ifndef CLSOCKETBASE_H
#define CLSOCKETBASE_H

#include <memory>
#include <string>
#include <sys/param.h>
#if defined(__WXOSX__) || defined(BSD)
#include <sys/errno.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#endif

#ifdef _WIN32
typedef SOCKET socket_t;
typedef int socklen_t;
#else
typedef int socket_t;
#define INVALID_SOCKET -1
#endif

using namespace std;
namespace dap
{
class Socket
{
protected:
    socket_t m_socket;
    bool m_closeOnExit;

public:
    typedef shared_ptr<Socket> Ptr_t;

    enum {
        kSuccess = 1,
        kTimeout = 2,
    };

#ifdef _WIN32
    static const int eWouldBlock = WSAEWOULDBLOCK;
#else
    static const int eWouldBlock = EWOULDBLOCK;
#endif

    static int GetLastError();
    static string error();
    static string error(const int errorCode);

public:
    /**
     * @brief set the socket into blocking/non-blocking mode
     * @param blocking
     */
    void MakeSocketBlocking(bool blocking);

    Socket(socket_t sockfd = INVALID_SOCKET);
    virtual ~Socket();

    void SetCloseOnExit(bool closeOnExit) { this->m_closeOnExit = closeOnExit; }
    bool IsCloseOnExit() const { return m_closeOnExit; }
    /**
     * @brief return the descriptor and clear this socket.
     */
    socket_t Release();

    /**
     * @brief initialize the socket library
     */
    static void Initialize();

    /**
     * @brief return platform specific socket handle
     */
    socket_t GetSocket() const { return m_socket; }

    /**
     * @brief send message. This function blocks until the entire buffer is sent
     * @throws SocketException
     */
    void Send(const string& msg);

    /**
     * @brief
     * @param timeout milliseconds to wait
     * @return kSuccess or kTimeout
     * @throws SocketException
     */
    int Read(char* buffer, size_t bufferSize, size_t& bytesRead);

    /**
     * @brief read string content from remote server
     * @param content [output]
     * @return kSuccess or kTimeout
     * @throws SocketException
     */
    int Read(string& content);

    /**
     * @brief select for read. Same as above, but use milli seconds instead
     * @param milliSeconds number of _milliseconds_ to wait
     * @return kSuccess or kTimeout
     * @throws SocketException
     */
    int SelectReadMS(long milliSeconds);


    /**
     * @brief select for write (milli seconds version)
     * @return kSuccess or kTimeout
     * @throws SocketException
     */
    int SelectWriteMS(long milliSeconds = -1);

    template <typename T>
    T* As() const
    {
        return dynamic_cast<T*>(const_cast<Socket*>(this));
    }

protected:
    /**
     * @brief
     */
    void DestroySocket();
};
};     // namespace dap
#endif // CLSOCKETBASE_H
