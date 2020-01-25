#include "Process.hpp"
#include <iostream>

void dap::Process::StartReaderThread()
{
    m_shutdown.store(false);
    m_readerThread = new thread(
        [](dap::Process* process, Queue<pair<string, string>>& Q, atomic_bool& shutdown) {
            string stdoutBuff;
            string stderrBuff;
            while(!shutdown.load()) {
                stdoutBuff.clear();
                stderrBuff.clear();
                bool readSuccess = process->DoRead(stdoutBuff, stderrBuff);
                bool readSomething = (!stdoutBuff.empty() || !stderrBuff.empty());
                if(readSomething && readSuccess) {
                    Q.push({ stdoutBuff, stderrBuff });
                } else {
                    this_thread::sleep_for(chrono::milliseconds(10));
                }
            }
            cout << "Going down" << endl;
        },
        this, ref(m_inQueue), ref(m_shutdown));
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

pair<string, string> dap::Process::Read() { return m_inQueue.pop(chrono::milliseconds(1)); }
