#ifdef __WIN32__
#include "Process.hpp"
#include "StringUtils.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <windows.h>

using namespace std;
namespace dap
{
static bool CheckIsAlive(HANDLE hProcess)
{
    DWORD dwExitCode;
    if(GetExitCodeProcess(hProcess, &dwExitCode)) {
        if(dwExitCode == STILL_ACTIVE)
            return true;
    }
    return false;
}

template <typename T>
bool WriteStdin(const T& buffer, HANDLE hStdin, HANDLE hProcess)
{
    DWORD dwMode;

    // Make the pipe to non-blocking mode
    dwMode = PIPE_READMODE_BYTE | PIPE_NOWAIT;
    SetNamedPipeHandleState(hStdin, &dwMode, NULL, NULL);
    DWORD bytesLeft = buffer.length();
    long offset = 0;
    size_t retryCount = 0;
    while(bytesLeft > 0 && (retryCount < 100)) {
        DWORD dwWritten = 0;
        if(!WriteFile(hStdin, buffer.c_str() + offset, bytesLeft, &dwWritten, NULL)) {
            return false;
        }
        if(!CheckIsAlive(hProcess)) {
            return false;
        }
        if(dwWritten == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        bytesLeft -= dwWritten;
        offset += dwWritten;
        ++retryCount;
    }
    return true;
}

#define CLOSE_HANDLE(h)             \
    if(h != INVALID_HANDLE_VALUE) { \
        CloseHandle(h);             \
    }                               \
    h = INVALID_HANDLE_VALUE;

class ProcessMSW : public Process
{
public:
    HANDLE m_stdinRead = INVALID_HANDLE_VALUE;
    HANDLE m_stdinWrite = INVALID_HANDLE_VALUE;
    HANDLE m_stdoutWrite = INVALID_HANDLE_VALUE;
    HANDLE m_stdoutRead = INVALID_HANDLE_VALUE;
    HANDLE m_stderrWrite = INVALID_HANDLE_VALUE;
    HANDLE m_stderrRead = INVALID_HANDLE_VALUE;

    DWORD m_dwProcessId = -1;
    PROCESS_INFORMATION m_piProcInfo;
    char m_buffer[65537];

protected:
    bool DoReadFromPipe(HANDLE pipe, string& buff);
    bool DoRead(string& ostrout, string& ostrerr);

public:
    ProcessMSW() {}
    virtual ~ProcessMSW() { Cleanup(); }
    bool Write(const string& str);
    bool IsAlive() const;
    void Cleanup();
    void Terminate();
};

void ProcessMSW::Cleanup()
{
    Process::Cleanup();
    if(IsAlive()) {
        TerminateProcess(m_piProcInfo.hProcess, 255);
    }
    CLOSE_HANDLE(m_piProcInfo.hProcess);
    CLOSE_HANDLE(m_piProcInfo.hThread);
    CLOSE_HANDLE(m_stdinRead);
    CLOSE_HANDLE(m_stdinWrite);
    CLOSE_HANDLE(m_stdoutWrite);
    CLOSE_HANDLE(m_stdoutRead);
    CLOSE_HANDLE(m_stderrWrite);
    CLOSE_HANDLE(m_stderrRead);
}

bool ProcessMSW::DoRead(string& ostrout, string& ostrerr)
{
    return IsAlive() && (DoReadFromPipe(m_stdoutRead, ostrout) || DoReadFromPipe(m_stderrRead, ostrerr));
}

Process* ExecuteProcess(const string& cmd, const string& workingDir)
{
    UNUSED(workingDir);
    // Set the bInheritHandle flag so pipe handles are inherited.
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    ProcessMSW* prc = new ProcessMSW();
    PROCESS_INFORMATION& process_info = prc->m_piProcInfo;

    // Save the handle to the current STDOUT.
    HANDLE savedStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE savedStderr = GetStdHandle(STD_ERROR_HANDLE);
    HANDLE savedStdin = GetStdHandle(STD_INPUT_HANDLE);

    // Create a pipe for the child process's STDOUT.
    if(!CreatePipe(&prc->m_stdoutRead, &prc->m_stdoutWrite, &saAttr, 0)) {
        delete prc;
        return nullptr;
    }

    // Create a pipe for the child process's STDERR.
    if(!CreatePipe(&prc->m_stderrRead, &prc->m_stderrWrite, &saAttr, 0)) {
        delete prc;
        return NULL;
    }
    // Create a pipe for the child process's STDIN.
    if(!CreatePipe(&prc->m_stdinRead, &prc->m_stdinWrite, &saAttr, 0)) {
        delete prc;
        return NULL;
    }

    // Execute the child process
    STARTUPINFO startup_info;
    ZeroMemory(&process_info, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&startup_info, sizeof(STARTUPINFO));

    startup_info.cb = sizeof(STARTUPINFO);
    startup_info.hStdInput = prc->m_stdinRead;
    startup_info.hStdOutput = prc->m_stdoutWrite;
    startup_info.hStdError = prc->m_stderrWrite;
    startup_info.dwFlags |= STARTF_USESTDHANDLES;
    BOOL ret = CreateProcess(NULL,
                             (char*)cmd.c_str(),  // shell line execution command
                             NULL,                // process security attributes
                             NULL,                // primary thread security attributes
                             TRUE,                // handles are inherited
                             0,                   // creation flags
                             NULL,                // use parent's environment
                             NULL,                // CD to tmp dir
                             &startup_info,       // STARTUPINFO pointer
                             &prc->m_piProcInfo); // receives PROCESS_INFORMATION
    if(ret) {
        prc->m_dwProcessId = prc->m_piProcInfo.dwProcessId;
    } else {
        delete prc;
        return NULL;
    }
    prc->StartReaderThread();
    return prc;
}

bool ProcessMSW::DoReadFromPipe(HANDLE pipe, string& buff)
{
    DWORD dwRead;
    DWORD dwMode;
    DWORD dwTimeout;

    // Make the pipe to non-blocking mode
    dwMode = PIPE_READMODE_BYTE | PIPE_NOWAIT;
    dwTimeout = 1000;
    SetNamedPipeHandleState(pipe, &dwMode, NULL, &dwTimeout);

    bool read_something = false;
    while(true) {
        BOOL bRes = ReadFile(pipe, m_buffer, sizeof(m_buffer) - 1, &dwRead, NULL);
        if(bRes) {
            string tmpBuff;
            // Success read
            m_buffer[dwRead / sizeof(char)] = 0;
            tmpBuff = m_buffer;
            buff += tmpBuff;
            read_something = true;
            continue;
        }
        break;
    }
    return read_something;
}

bool ProcessMSW::Write(const string& buff)
{
    DWORD dwMode;
    DWORD dwTimeout;
    char chBuf[4097];

    string tmpCmd = buff;
    StringUtils::Rtrim(tmpCmd);
    tmpCmd += "\n";
    strcpy(chBuf, tmpCmd.c_str());

    // Make the pipe to non-blocking mode
    dwMode = PIPE_READMODE_BYTE | PIPE_NOWAIT;
    dwTimeout = 30000;
    UNUSED(dwTimeout);
    SetNamedPipeHandleState(m_stdinWrite, &dwMode, NULL,
                            NULL); // Timeout of 30 seconds
    return WriteStdin(tmpCmd, m_stdinWrite, m_piProcInfo.hProcess);
}

bool ProcessMSW::IsAlive() const { return CheckIsAlive(m_piProcInfo.hProcess); }

void ProcessMSW::Terminate()
{
    // terminate and perform cleanup
    Cleanup();
}
};     // namespace dap
#endif //__WXMSW__
