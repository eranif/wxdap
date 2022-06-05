#ifndef PROCESS_H__
#define PROCESS_H__

#include "Queue.hpp"
#include <atomic>
#include <thread>
#include <wx/string.h>

namespace dap
{
class Process
{
    Queue<std::pair<wxString, wxString>> m_inQueue;
    std::thread* m_readerThread = nullptr;
    std::atomic_bool m_shutdown;

protected:
    /**
     * @brief impleemnt the actual read call. This method is not accessible outside of this class
     */
    virtual bool DoRead(wxString& str, wxString& err_buff) = 0;

public:
    /**
     * @brief launch a background thread that will perform the reading from the process
     */
    void StartReaderThread();

public:
    Process() {}
    virtual ~Process() {}

    virtual bool Write(const wxString& str) = 0;
    virtual bool WriteLn(const wxString& str) = 0;
    virtual bool IsAlive() const = 0;
    virtual void Cleanup();
    virtual void Terminate() = 0;
    /**
     * @brief this method checks the queue and returns immediately (up to 1 millisecond)
     * @return pair of strings. first is stdout, second is stderr
     */
    std::pair<wxString, wxString> Read();
};

/**
 * @brief Create process and return the handle to it
 * @param cmd process command
 * @param workingDir process's working directory
 * @return pointer to Process object
 */
Process* ExecuteProcess(const wxString& cmd, // Command Line
                        const wxString& workingDir = ".");

};     // namespace dap
#endif // PROCESS_H__
