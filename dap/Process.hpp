#ifndef PROCESS_H__
#define PROCESS_H__

#include "Queue.hpp"
#include <atomic>
#include <string>
#include <thread>

using namespace std;

namespace dap
{
class Process
{
    Queue<pair<string, string>> m_inQueue;
    thread* m_readerThread = nullptr;
    atomic_bool m_shutdown;

protected:
    /**
     * @brief impleemnt the actual read call. This method is not accessible outside of this class
     */
    virtual bool DoRead(string& str, string& err_buff) = 0;

public:
    /**
     * @brief launch a background thread that will perform the reading from the process
     */
    void StartReaderThread();

public:
    Process() {}
    virtual ~Process() {}

    virtual bool Write(const string& str) = 0;
    virtual bool IsAlive() const = 0;
    virtual void Cleanup();
    virtual void Terminate() = 0;
    /**
     * @brief this method checks the queue and returns immediately (up to 1 millisecond)
     */
    bool Read(string& str, string& err_buff);
};

/**
 * @brief Create process and return the handle to it
 * @param cmd process command
 * @param workingDir process's working directory
 * @return pointer to Process object
 */
Process* ExecuteProcess(const string& cmd, // Command Line
                        const string& workingDir = ".");

};     // namespace dap
#endif // PROCESS_H__
