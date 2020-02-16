#include "GDBMI.hpp"
#include "GdbHandler.hpp"
#include "dap/Log.hpp"
#include "dap/StringUtils.hpp"
#include "utils.hpp"
#include <iomanip>
#include <sstream>

GdbHandler::GdbHandler() {}

GdbHandler::~GdbHandler() { DELETE_PTR(m_process); }

string GdbHandler::ParseErrorMessage(const string& output)
{
    // ^error,msg="C:UsersEranDocumentsAmitTestbuild-DebugbinAmitTest.exe: No such file or directory."
    string errorMessage = StringUtils::AfterFirst(output, '"');
    errorMessage = StringUtils::BeforeFirst(errorMessage, '"');
    return errorMessage;
}

void GdbHandler::OnLaunchRequest(dap::ProtocolMessage::Ptr_t message)
{
    dap::LaunchRequest* req = message->As<dap::LaunchRequest>();
    if(req->arguments.debuggee.empty()) {
        throw dap::Exception("Empty debugee arguments");
    }

    // Keep the args
    int requestSeq = req->seq;
    m_debugeeArgs = req->arguments.debuggee;
    string debuggee_executable = m_debugeeArgs[0];
    StringUtils::ToUnixPath(debuggee_executable);
    StringUtils::WrapWithQuotes(debuggee_executable);

    string fileCommand;
    string commandSequence = NextSequence();
    fileCommand << commandSequence << "-file-exec-and-symbols " << debuggee_executable;
    LOG_INFO() << "Loading file into gdb:" << fileCommand;
    m_process->WriteLn(fileCommand);
    m_handlersMap.insert({ commandSequence, [=](const string& output) -> dap::ProtocolMessage::Ptr_t {
                              // Process the output. Output is guranteed to be a complete reply from the debeugger
                              if(IsSuccess(output)) {
                                  LOG_INFO() << "File loaded successfully!";
                                  return nullptr;
                              } else if(IsError(output)) {
                                  return ResponseError<dap::LaunchResponse>(output, requestSeq);
                              }
                              return nullptr;
                          } });

    string setArgsCommand;
    commandSequence = NextSequence();
    setArgsCommand << commandSequence << "-exec-arguments ";
    for(const auto& arg : m_debugeeArgs) {
        string arg_ = StringUtils::WrapWithQuotes(arg);
        setArgsCommand << arg_ << " ";
    }

    LOG_INFO() << "Settings arguments:" << setArgsCommand;
    m_process->WriteLn(setArgsCommand);
    m_handlersMap.insert({ commandSequence, [=](const string& output) -> dap::ProtocolMessage::Ptr_t {
                              // Process the output. Output is guranteed to be a complete reply from the debeugger
                              if(IsSuccess(output)) {
                                  LOG_INFO() << "Arguments passed to the debuggee successfully";
                                  return nullptr;
                              } else if(IsError(output)) {
                                  return ResponseError<dap::LaunchResponse>(output, requestSeq);
                              }
                              return nullptr;
                          } });
    string runCommand;
    commandSequence = NextSequence();
    runCommand << commandSequence << "-exec-run";
    LOG_INFO() << "Starting the debuggee";
    m_process->WriteLn(runCommand);
    m_handlersMap.insert({ commandSequence, [=](const string& output) -> dap::ProtocolMessage::Ptr_t {
                              // Process the output. Output is guranteed to be a complete reply from the debeugger
                              if(IsSuccess(output) || IsRunning(output)) {
                                  // Time to report Launch response
                                  dap::LaunchResponse* response = new dap::LaunchResponse();
                                  response->success = true;
                                  response->request_seq = requestSeq;
                                  LOG_INFO() << "Debuggee started successfully (running)";
                                  return dap::ProtocolMessage::Ptr_t(response);
                              } else if(IsError(output)) {
                                  // error occured
                                  return ResponseError<dap::LaunchResponse>(output, requestSeq);
                              }
                              return nullptr;
                          } });
}

void GdbHandler::OnConfigurationDoneRequest(dap::ProtocolMessage::Ptr_t message)
{
    LOG_INFO() << "Received configurationDone request";
    dap::ConfigurationDoneRequest* req = message->As<dap::ConfigurationDoneRequest>();
    dap::ConfigurationDoneResponse* response = new dap::ConfigurationDoneResponse();
    response->success = true;
    response->request_seq = req->seq;
    PushMessage(dap::ProtocolMessage::Ptr_t(response));
    LOG_INFO() << "Sending configurationDone response";
}

void GdbHandler::OnSetBreakpoints(dap::ProtocolMessage::Ptr_t message)
{
    // Implentation:
    // Delete all breakpoints in the given source
    // and apply the new breakpoints
    dap::SetBreakpointsRequest* req = message->As<dap::SetBreakpointsRequest>();
    string path = StringUtils::ToUnixPath(req->arguments.source.path);

    // Delete all breakpoints for this line
    int requestSeq = req->seq;
    for(const auto& b : req->arguments.breakpoints) {
        string commandSequence = NextSequence();
        string command = commandSequence;
        string where;
        where << path << ":" << b.line;
        StringUtils::WrapWithQuotes(where);

        command << "-break-insert -f " << where;
        LOG_INFO() << "Setting breakpoint:" << command;
        m_process->WriteLn(command);
        m_handlersMap.insert({ commandSequence, [=](const string& output) -> dap::ProtocolMessage::Ptr_t {
                                  if(IsSuccess(output)) {
                                      auto bpt = GDBMI::ParseBreakpoint(output);
                                      dap::SetBreakpointsResponse* response = new dap::SetBreakpointsResponse();
                                      response->breakpoints.push_back(bpt);
                                      response->success = true;
                                      response->request_seq = requestSeq;
                                      LOG_INFO() << "Successfully set breakpoint #" << bpt.id << "->" << bpt.source.path
                                                 << ":" << bpt.line;
                                      return dap::ProtocolMessage::Ptr_t(response);
                                  } else if(StringUtils::StartsWith(output, "^error")) {
                                      return ResponseError<dap::SetBreakpointsResponse>(output, requestSeq);
                                  }
                                  return nullptr;
                              } });
    }
}
void GdbHandler::OnThreads(dap::ProtocolMessage::Ptr_t message)
{
    dap::ThreadsRequest* req = message->As<dap::ThreadsRequest>();
    int requestSeq = req->seq;
    string threadsInfoCommand;
    string commandSequence = NextSequence();
    threadsInfoCommand << commandSequence << "-thread-info";
    LOG_INFO() << "Listing threads";
    m_process->WriteLn(threadsInfoCommand);
    m_handlersMap.insert({ commandSequence, [=](const string& output) -> dap::ProtocolMessage::Ptr_t {
                              // Process the output. Output is guranteed to be a complete reply from the debeugger
                              if(IsSuccess(output) || IsRunning(output)) {
                                  // Time to report Launch response
                                  dap::ThreadsResponse* response = new dap::ThreadsResponse();
                                  response->success = true;
                                  response->request_seq = requestSeq;
                                  response->threads = GDBMI::ParseThreads(output);
                                  LOG_INFO() << "Listing threads successfully";
                                  return dap::ProtocolMessage::Ptr_t(response);
                              } else if(IsError(output)) {
                                  // error occured
                                  return ResponseError<dap::ThreadsResponse>(output, requestSeq);
                              }
                              return nullptr;
                          } });
}

void GdbHandler::StartDebugger(const string& debuggerExecutable, const string& wd)
{
    m_process = dap::ExecuteProcess(debuggerExecutable + " -i=mi", wd);
    if(!m_process) {
        throw dap::Exception("Failed to start debugger process: " + debuggerExecutable);
    }
    LOG_INFO() << "Started debugger:" << debuggerExecutable << "-i=mi";
}

string GdbHandler::NextSequence()
{
    stringstream ss;
    ss << setw(6) << setfill('0') << (++m_commandCounter);
    return ss.str();
}

void GdbHandler::OnDebuggerStdout(const string& message)
{
    if(!message.empty()) {
        m_stdout += message;
    }
    OnOutput(m_stdout);
}

void GdbHandler::OnDebuggerStderr(const string& message)
{
    if(!message.empty()) {
        m_stderr += message;
    }
    OnOutput(m_stderr);
}

void GdbHandler::OnHandleStateChange(const string& buffer)
{
    if(StringUtils::StartsWith(buffer, "*stopped")) {
        // Report debugger stopped event
        GDBMI::eStoppedReason reason = GDBMI::ParseStoppedReason(buffer);
        if(reason == GDBMI::kBreakpointHit) {
            LOG_INFO() << "Breakpoint hit";
            // Send breakpoint hit event
            dap::StoppedEvent* stopEvent = new dap::StoppedEvent();
            stopEvent->reason = "breakpoint";
            stopEvent->text = "Breakpoint hit";
            PushMessage(dap::ProtocolMessage::Ptr_t(stopEvent));
        }
    }
}

void GdbHandler::OnEvent(const string& buffer)
{
    if(StringUtils::StartsWith(buffer, "=breakpoint-modified")) {
        // Need to update about this breakpoint to the frontend
        PushMessage(OnBreakpointUpdate(buffer));
    }
}

void GdbHandler::OnOutput(string& inbuffer)
{
    // MI is line based
    while(true) {
        size_t where = inbuffer.find('\n');
        if(where == string::npos) {
            break;
        }
        string buffer = inbuffer.substr(0, where);
        StringUtils::Trim(buffer);
        inbuffer.erase(0, where + 1); // skip "\n"
        LOG_DEBUG() << "Processing line input from GDB:" << buffer;
        switch(buffer[0]) {
        case '=':
            // notify-async-output contains supplementary information that the client should handle (e.g., a new
            // breakpoint information). All notify output is prefixed by '='
            OnEvent(buffer);
            break;
        case '+':
            // status-async-output contains on-going status information about the progress of a slow operation. It can
            // be discarded. All status output is prefixed by '+'
            break;
        case '*':
            // exec-async-output contains asynchronous state change on the target (stopped, started, disappeared). All
            // async output is prefixed by '*'
            OnHandleStateChange(buffer);
            break;
        case '@':
            // target-stream-output is the output produced by the target program. All the target output is prefixed by
            // '@'
            LOG_DEBUG() << buffer;
            PushMessage(CreateOutputEvent(buffer));
            break;
        case '&':
            // log-stream-output is output text coming from GDB's internals, for instance messages that should be
            // displayed as part of an error log. All the log output is prefixed by '&'
            // We discard this
            break;
        case '~': {
            // console stream output
            LOG_DEBUG() << buffer;
            PushMessage(CreateOutputEvent(buffer));
        } break;
        default: {
            if(buffer.length() >= 6) {
                string seq = buffer.substr(0, 6);
                buffer.erase(0, 6);
                auto iter = m_handlersMap.find(seq);
                if(iter != m_handlersMap.end()) {
                    PushMessage(iter->second(buffer));
                } else {
                    PushMessage(CreateOutputEvent(buffer));
                }
            } else {
                PushMessage(CreateOutputEvent(buffer));
            }
        } break;
        }
    }
}

dap::ProtocolMessage::Ptr_t GdbHandler::CreateOutputEvent(const string& buffer)
{
    dap::OutputEvent* outputEvent = new dap::OutputEvent();
    outputEvent->output = buffer;
    outputEvent->category = "console";
    return dap::ProtocolMessage::Ptr_t(outputEvent);
}

dap::ProtocolMessage::Ptr_t GdbHandler::OnBreakpointUpdate(const string& buffer)
{
    auto bp = GDBMI::ParseBreakpoint(buffer);
    if(bp.verified) {
        dap::BreakpointEvent* breakpointEvent = new dap::BreakpointEvent();
        breakpointEvent->breakpoint = bp;
        breakpointEvent->reason = "breakpoint modified";
        LOG_INFO() << "Breakpoint modified #" << bp.id << "->" << bp.source.path << ":" << bp.line;
        return dap::ProtocolMessage::Ptr_t(breakpointEvent);
    }
    return nullptr;
}

bool GdbHandler::IsSuccess(const string& gdbOutput) { return StringUtils::StartsWith(gdbOutput, "^done"); }

bool GdbHandler::IsError(const string& gdbOutput) { return StringUtils::StartsWith(gdbOutput, "^error"); }

bool GdbHandler::IsRunning(const string& gdbOutput) { return StringUtils::StartsWith(gdbOutput, "^running"); }
