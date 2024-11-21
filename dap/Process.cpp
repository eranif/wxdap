#include "Process.hpp"

#include "Log.hpp"

void dap::Process::StartThreads()
{
    m_shutdown.store(false);
    m_readerThread = new std::thread(
        [](dap::Process* process, Queue<std::string>& outq, Queue<std::string>& errq, std::atomic_bool& shutdown) {
            while (!shutdown.load()) {
                std::string stdoutBuff;
                std::string stderrBuff;
                bool readSuccess = process->DoRead(stdoutBuff, stderrBuff);
                bool readSomething = (!stdoutBuff.empty() || !stderrBuff.empty());
                if (readSomething && readSuccess) {
                    if (!stdoutBuff.empty()) {
                        outq.push(stdoutBuff);
                    }
                    if (!stderrBuff.empty()) {
                        errq.push(stderrBuff);
                    }
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }
            }
            LOG_ERROR() << "Going down";
        },
        this, std::ref(m_stdoutQueue), std::ref(m_stderrQueue), std::ref(m_shutdown));

    m_isAliveThread = new std::thread(
        [](dap::Process* process, std::atomic_bool& shutdown) {
            while (process->IsAlive() && !shutdown.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            // if we reached here, we the process terminated, fire event, set the shutdown flag
            // and exit
            shutdown.store(true);
            LOG_ERROR() << "Process terminated." << endl;
        },
        this, std::ref(m_shutdown));
}

dap::Process::~Process() {}

void dap::Process::Cleanup()
{
    m_shutdown = true;
    if (m_readerThread) {
        m_readerThread->join();
    }

    if (m_isAliveThread) {
        m_isAliveThread->join();
    }
    wxDELETE(m_readerThread);
    wxDELETE(m_isAliveThread);
    m_shutdown = false;
}

std::optional<std::string> dap::Process::ReadStdout(int timeout_ms)
{
    return m_stdoutQueue.pop(std::chrono::milliseconds(timeout_ms));
}

std::optional<std::string> dap::Process::ReadStderr(int timeout_ms)
{
    return m_stderrQueue.pop(std::chrono::milliseconds(timeout_ms));
}
