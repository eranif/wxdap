#include "UnixProcess.hpp"
#if defined(__linux__)
#include "Log.hpp"
#include <csignal>
#include <cstring>
#include <sys/select.h>
#include <sys/types.h>

UnixProcess::UnixProcess(const vector<string>& args)
{
    m_goingDown.store(false);

    // Open the pipes
    if(!m_childStdin.Open() || !m_childStderr.Open() || !m_childStdout.Open()) {
        LOG_ERROR() << "Could not open redirection pipes." << strerror(errno);
        return;
    }

    child_pid = fork();
    if(child_pid == -1) {
        LOG_ERROR() << "Failed to start child process" << strerror(errno);
    }
    if(child_pid == 0) {
        // In child process
        dup2(m_childStdin.GetReadFd(), STDIN_FILENO);
        dup2(m_childStdout.GetWriteFd(), STDOUT_FILENO);
        dup2(m_childStderr.GetWriteFd(), STDERR_FILENO);
        m_childStdin.Close();
        m_childStdout.Close();
        m_childStderr.Close();

        char** argv = new char*[args.size() + 1];
        for(size_t i = 0; i < args.size(); ++i) {
            const string& arg = args[i];
            argv[i] = new char[arg.length() + 1];
            strcpy(argv[i], arg.c_str());
            argv[i][arg.length()] = 0;
        }
        argv[args.size()] = 0;
        int result = execvp(argv[0], const_cast<char* const*>(argv));
        int errNo = errno;
        if(result == -1) {
            // Note: no point writing to stdout here, it has been redirected
            LOG_ERROR() << "Error: Failed to launch program:" << args;
            exit(EXIT_FAILURE);
        }
    } else {
        // parent process
        m_childStdin.CloseReadFd();
        m_childStdout.CloseWriteFd();
        m_childStderr.CloseWriteFd();
    }
}

UnixProcess::~UnixProcess()
{
    // Kill the child process (if it is still alive)
    Terminate();
}

#define CHUNK_SIZE 1024
#define MAX_BUFF_SIZE (1024 * 2048)
bool UnixProcess::ReadAll(int fd, string& content, int timeoutMilliseconds)
{
    fd_set rset;
    char buff[CHUNK_SIZE];
    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    int seconds = timeoutMilliseconds / 1000;
    int ms = timeoutMilliseconds % 1000;

    struct timeval tv = { seconds, ms * 1000 }; //  10 milliseconds timeout
    while(true) {
        int rc = ::select(fd + 1, &rset, nullptr, nullptr, &tv);
        if(rc > 0) {
            int len = read(fd, buff, (sizeof(buff) - 1));
            if(len > 0) {
                buff[len] = 0;
                content.append(buff);
                if(content.length() >= MAX_BUFF_SIZE) {
                    return true;
                }
                // clear the tv struct so next select() call will return immediately
                tv.tv_usec = 0;
                tv.tv_sec = 0;
                FD_ZERO(&rset);
                FD_SET(fd, &rset);
                continue;
            }
        } else if(rc == 0) {
            // timeout
            return true;
        }
        break;
    }
    // error
    return false;
}

bool UnixProcess::Write(int fd, const string& message, atomic_bool& shutdown)
{
    int bytes = 0;
    string tmp = message;
    const int chunkSize = 4096;
    while(!tmp.empty() && !shutdown.load()) {
        errno = 0;
        bytes = ::write(fd, tmp.c_str(), tmp.length() > chunkSize ? chunkSize : tmp.length());
        int errCode = errno;
        if(bytes < 0) {
            if((errCode == EWOULDBLOCK) || (errCode == EAGAIN)) {
                this_thread::sleep_for(chrono::milliseconds(10));
            } else if(errCode == EINTR) {
                continue;
            } else {
                break;
            }
        } else if(bytes) {
            tmp.erase(0, bytes);
        }
    }
    LOG_DEBUG() << "Wrote message of size:" << message.length();
    return tmp.empty();
}

int UnixProcess::Wait()
{
    if(child_pid != -1) {
        int status = 0;
        waitpid(child_pid, &status, WNOHANG);
        return WEXITSTATUS(status);
    } else {
        return 0;
    }
}

void UnixProcess::Stop()
{
    if(child_pid != -1) {
        ::kill(child_pid, SIGTERM);
    }
}

bool UnixProcess::Write(const string& message)
{
    return UnixProcess::Write(m_childStdin.GetWriteFd(), message, m_goingDown);
}

bool UnixProcess::DoRead(string& str, string& err_buff)
{
    if(!IsAlive()) {
        return false;
    }
    ReadAll(m_childStdout.GetReadFd(), str, 10);
    ReadAll(m_childStderr.GetReadFd(), err_buff, 10);
    return !str.empty() || !err_buff.empty();
}

bool UnixProcess::IsAlive() const { return (::kill(child_pid, 0) == 0); }

void UnixProcess::Terminate()
{
    Stop();
    Wait();
}

#endif // OSX & GTK
