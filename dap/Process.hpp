#ifndef PROCESS_H__
#define PROCESS_H__

#include "Queue.hpp"
#include "dap_exports.hpp"

#include <atomic>
#include <thread>
#include <wx/event.h>
#include <wx/process.h>
#include <wx/string.h>

namespace dap
{
class WXDLLIMPEXP_DAP Process
{
public:
    /**
     * @brief launch a background thread that will perform the reading from the process
     */
    void StartThreads();

    Process() {}
    virtual ~Process();

    virtual bool Write(const std::string& str) = 0;
    virtual bool WriteLn(const std::string& str) = 0;
    virtual bool IsAlive() const = 0;
    virtual void Terminate() = 0;
    virtual void Cleanup();

    void SetProcessId(int processId) { this->m_processId = processId; }
    int GetProcessId() const { return m_processId; }

    std::optional<std::string> ReadStdout(int timeout_ms);
    std::optional<std::string> ReadStderr(int timeout_ms);

protected:
    /**
     * @brief implement the actual read call. This method is not accessible outside of this class
     */
    virtual bool DoRead(std::string& str, std::string& err_buff) = 0;

private:
    std::thread* m_readerThread = nullptr;
    std::thread* m_isAliveThread = nullptr;
    std::atomic_bool m_shutdown;
    int m_processId = wxNOT_FOUND;
    Queue<std::string> m_stdoutQueue;
    Queue<std::string> m_stderrQueue;
};

/**
 * @brief Create process and return the handle to it
 * @param cmd process command
 * @param workingDir process's working directory
 * @return pointer to Process object
 */
WXDLLIMPEXP_DAP Process* ExecuteProcess(const wxString& cmd, // Command Line
                                        const wxString& workingDir = ".");

}; // namespace dap
#endif // PROCESS_H__
