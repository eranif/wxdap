#include "Process.hpp"

#include "Log.hpp"

#include <iostream>

void dap::Process::StartReaderThread()
{
    m_shutdown.store(false);
    m_readerThread = new std::thread(
        [](dap::Process* process, Queue<std::pair<wxString, wxString>>& Q, std::atomic_bool& shutdown) {
            wxString stdoutBuff;
            wxString stderrBuff;
            while(!shutdown.load()) {
                stdoutBuff.clear();
                stderrBuff.clear();
                bool readSuccess = process->DoRead(stdoutBuff, stderrBuff);
                bool readSomething = (!stdoutBuff.empty() || !stderrBuff.empty());
                if(readSomething && readSuccess) {
                    Q.push({ stdoutBuff, stderrBuff });
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            LOG_ERROR() << "Going down";
        },
        this, std::ref(m_inQueue), std::ref(m_shutdown));
}

void dap::Process::Cleanup()
{
    if(m_readerThread) {
        m_shutdown.store(true);
        m_readerThread->join();
    }
    delete m_readerThread;
    m_readerThread = nullptr;
}

std::pair<wxString, wxString> dap::Process::Read() { return m_inQueue.pop(std::chrono::milliseconds(1)); }
