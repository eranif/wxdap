#ifndef UNIX_PROCESS_H
#define UNIX_PROCESS_H
#if defined(__linux__)
#include "Process.hpp"
#include "Queue.hpp"

#include <atomic>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <wx/string.h>

// Wrapping pipe in a class makes sure they are closed when we leave scope
#define CLOSE_FD(fd) \
    if(fd != -1) {   \
        ::close(fd); \
        fd = -1;     \
    }

using namespace std;
class CPipe
{
private:
    int m_readFd = -1;
    int m_writeFd = -1;

public:
    const inline int GetReadFd() const { return m_readFd; }
    const inline int GetWriteFd() const { return m_writeFd; }
    CPipe() {}
    void Close()
    {
        CLOSE_FD(m_readFd);
        CLOSE_FD(m_writeFd);
    }
    ~CPipe() { Close(); }
    bool Open()
    {
        int fd[2];
        if(pipe(fd) == 0) {
            m_readFd = fd[0];
            m_writeFd = fd[1];
            return true;
        }
        return false;
    }
    void CloseWriteFd() { CLOSE_FD(m_writeFd); }
    void CloseReadFd() { CLOSE_FD(m_readFd); }
};

class UnixProcess : public dap::Process
{
private:
    CPipe m_childStdin;
    CPipe m_childStdout;
    CPipe m_childStderr;
    atomic_bool m_goingDown;
    wxString m_stdout;
    wxString m_stderr;
    int m_pid = wxNOT_FOUND;

protected:
    // sync operations
    static bool ReadAll(int fd, wxString& content, int timeoutMilliseconds);
    static bool Write(int fd, const wxString& message, atomic_bool& shutdown);

    bool DoRead(wxString& str, wxString& err_buff) override;

public:
    int child_pid = -1;

    UnixProcess(const vector<wxString>& args);
    virtual ~UnixProcess();

    // wait for process termination
    int Wait();

    // Write to the process
    bool Write(const wxString& message) override;

    // Same as Write, but add LF at the end of the message
    bool WriteLn(const wxString& message) override;

    // stop the running process
    void Stop();

    /**
     * @brief is the process still alive?
     */
    bool IsAlive() const override;

    /**
     * @brief terminate the process
     */
    void Terminate() override;
};
#endif // defined(__linux__)
#endif // UNIX_PROCESS_H
