#if defined(__APPLE__) || defined(__linux__)

#include "Log.hpp"
#include "Process.hpp"
#include "StringUtils.hpp"
#include "UnixProcess.hpp"

#include <vector>
#include <wx/string.h>

namespace dap
{
Process* ExecuteProcess(const wxString& cmd, const wxString& workingDir)
{
    std::vector<wxString> args = StringUtils::BuildArgv(cmd);
    LOG_DEBUG() << "Starting process:" << args;
    UnixProcess* process = new UnixProcess(args);
    process->StartReaderThread();
    process->SetProcessId(process->child_pid);
    return process;
}

}; // namespace dap
#endif