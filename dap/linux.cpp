#ifdef __linux__

#include "Process.hpp"
#include "UnixProcess.hpp"
#include "StringUtils.hpp"
#include "Log.hpp"

namespace dap
{
Process* ExecuteProcess(const string& cmd, const string& workingDir) 
{
    vector<string> args = StringUtils::BuildArgv(cmd);
    LOG_DEBUG() << "Starting process:" << args;
    UnixProcess* process = new UnixProcess(args);
    process->StartReaderThread();
    return process;
}

}; // namespace dap
#endif