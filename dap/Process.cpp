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

bool dap::Process::Read(string& str, string& err_buff)
{
    pair<string, string> buff;
    if(m_inQueue.pop(buff, chrono::milliseconds(1))) {
        str = buff.first;
        err_buff = buff.second;
        return true;
    }
    return false;
}
