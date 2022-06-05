#include "Socket.hpp"
#include "Exception.hpp"
#include <cerrno>
#include <cstdio>
#include <memory>
#include <sstream>

#ifndef _WIN32
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif
namespace dap
{
Socket::Socket(socket_t sockfd)
    : m_socket(sockfd)
    , m_closeOnExit(true)
{
    if(m_socket != INVALID_SOCKET) {
        MakeSocketBlocking(false);
    }
}

Socket::~Socket() { DestroySocket(); }

void Socket::Initialize()
{
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
}

int Socket::Read(wxString& content)
{
    char buffer[4096];
    size_t bytesRead = 0;
    int rc = Read(buffer, sizeof(buffer) - 1, bytesRead);
    if(rc == kSuccess) {
        buffer[bytesRead] = 0;
        content.reserve(bytesRead + 1);
        content = buffer;
    }
    return rc;
}

int Socket::Read(char* buffer, size_t bufferSize, size_t& bytesRead)
{
    int res = recv(m_socket, buffer, bufferSize, 0);
    if(res < 0) {
        int err = GetLastError();
        if(eWouldBlock == err) {
            return kTimeout;
        }
        throw Exception("Read failed: " + error(err));
    } else if(0 == res) {
        throw Exception("Read failed: " + error());
    }

    bytesRead = static_cast<size_t>(res);
    return kSuccess;
}

// Send API
void Socket::Send(const wxString& msg)
{
    if(m_socket == INVALID_SOCKET) {
        throw Exception("Invalid socket!");
    }
    if(msg.empty()) {
        return;
    }

    auto buffer = msg.mb_str(wxConvUTF8);
    char* pdata = buffer.data();
    int bytesLeft = msg.length();
    while(bytesLeft) {
        if(SelectWriteMS(1000) == kTimeout)
            continue;
        int bytesSent = ::send(m_socket, (const char*)pdata, bytesLeft, 0);
        if(bytesSent <= 0)
            throw Exception("Send error: " + error());
        pdata += bytesSent;
        bytesLeft -= bytesSent;
    }
}

int Socket::GetLastError()
{
#ifdef _WIN32
    return ::WSAGetLastError();
#else
    return errno;
#endif
}

wxString Socket::error() { return error(GetLastError()); }

wxString Socket::error(const int errorCode)
{
    wxString err;
#ifdef _WIN32
    // Get the error message, if any.
    if(errorCode == 0)
        return "No error message has been recorded";

    LPSTR messageBuffer = nullptr;
    size_t size =
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    wxString message(messageBuffer, size);

    // Free the buffer.
    LocalFree(messageBuffer);
    err = message;
#else
    err = strerror(errorCode);
#endif
    return err;
}

void Socket::DestroySocket()
{
    if(IsCloseOnExit()) {
        if(m_socket != INVALID_SOCKET) {
#ifdef _WIN32
            ::shutdown(m_socket, 2);
            ::closesocket(m_socket);
#else
            ::shutdown(m_socket, 2);
            ::close(m_socket);
#endif
        }
    }
    m_socket = INVALID_SOCKET;
}

socket_t Socket::Release()
{
    int fd = m_socket;
    m_socket = INVALID_SOCKET;
    return fd;
}

void Socket::MakeSocketBlocking(bool blocking)
{
#ifndef _WIN32
    // set socket to non-blocking mode
    int flags;
    flags = ::fcntl(m_socket, F_GETFL);
    if(blocking) {
        flags &= ~O_NONBLOCK;
    } else {
        flags |= O_NONBLOCK;
    }
    ::fcntl(m_socket, F_SETFL, flags);
#else
    u_long iMode = blocking ? 0 : 1;
    ::ioctlsocket(m_socket, FIONBIO, &iMode);
#endif
}

int Socket::SelectWriteMS(long milliSeconds)
{
    if(milliSeconds < 0) {
        throw Exception("Invalid timeout");
    }

    if(m_socket == INVALID_SOCKET) {
        throw Exception("Invalid socket!");
    }

    struct timeval tv = { milliSeconds / 1000, (milliSeconds % 1000) * 1000 };
    fd_set write_set;
    FD_ZERO(&write_set);
    FD_SET(m_socket, &write_set);
    errno = 0;
    int rc = select(m_socket + 1, NULL, &write_set, NULL, &tv);
    if(rc == 0) {
        // timeout
        return kTimeout;

    } else if(rc < 0) {
        // an error occurred
        throw Exception("SelectWriteMS failed: " + error());

    } else {
        // we got something to read
        return kSuccess;
    }
}

int Socket::SelectReadMS(long milliSeconds)
{
    if(milliSeconds < 0) {
        throw Exception("Invalid timeout");
    }

    if(m_socket == INVALID_SOCKET) {
        throw Exception("Invalid socket!");
    }
    int seconds = milliSeconds / 1000; // convert the number into seconds
    int ms = milliSeconds % 1000;      // the remainder is less than a second
    struct timeval tv = { seconds, ms * 1000 };

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(m_socket, &readfds);
    int rc = select(m_socket + 1, &readfds, NULL, NULL, &tv);
    if(rc == 0) {
        // timeout
        return kTimeout;

    } else if(rc < 0) {
        // an error occurred
        throw Exception("SelectRead failed: " + error());

    } else {
        // we got something to read
        return kSuccess;
    }
}
}; // namespace dap