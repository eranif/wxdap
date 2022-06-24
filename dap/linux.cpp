#ifdef __linux__

#include "Log.hpp"
#include "Process.hpp"
#include "StringUtils.hpp"
#include "UnixProcess.hpp"

#include <wx/string.h>

namespace dap
{
Process* ExecuteProcess(const wxString& cmd, const wxString& workingDir)
{
    vector<wxString> args = StringUtils::BuildArgv(cmd);
    LOG_DEBUG() << "Starting process:" << args;
    UnixProcess* process = new UnixProcess(args);
    process->StartReaderThread();
    process->SetProcessId(process->child_pid);
    return process;
}

}; // namespace dap
#endif