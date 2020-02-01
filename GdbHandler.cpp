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
    stringstream ss;
    string debuggee_executable = m_debugeeArgs[0];
    StringUtils::ToUnixPath(debuggee_executable);
    StringUtils::WrapWithQuotes(debuggee_executable);

    string commandSequence = NextSequence();
    ss << commandSequence << "-file-exec-and-symbols " << debuggee_executable;
    LOG_DEBUG() << ss.str();
    m_process->WriteLn(ss.str());
    m_handlersMap.insert({ commandSequence, [=](const string& output) -> dap::ProtocolMessage::Ptr_t {
                              // Process the output. Output is guranteed to be a complete reply from the debeugger
                              if(StringUtils::StartsWith(output, "^done")) {
                                  dap::LaunchResponse* response = new dap::LaunchResponse();
                                  response->success = true;
                                  response->request_seq = requestSeq;
                                  return dap::ProtocolMessage::Ptr_t(response);
                              } else if(StringUtils::StartsWith(output, "^error")) {
                                  dap::LaunchResponse* response = new dap::LaunchResponse();
                                  response->success = false;
                                  response->message = GdbHandler::ParseErrorMessage(output);
                                  response->request_seq = requestSeq;
                                  LOG_ERROR() << "Sending error response:" << response->message;
                                  return dap::ProtocolMessage::Ptr_t(response);
                              }
                              return nullptr;
                          } });
}

void GdbHandler::OnSetBreakpoints(dap::ProtocolMessage::Ptr_t message)
{
    // Implentation:
    // Delete all breakpoints in the given source
    // and apply the new breakpoints
    dap::SetBreakpointsRequest* req = message->As<dap::SetBreakpointsRequest>();
    const string& path = req->arguments.source.path;
    
    // Delete all breakpoints for this line
    
//
//    // Keep the args
//    int requestSeq = req->seq;
//    m_debugeeArgs = req->arguments.debuggee;
//    stringstream ss;
//    string debuggee_executable = m_debugeeArgs[0];
//    StringUtils::ToUnixPath(debuggee_executable);
//    StringUtils::WrapWithQuotes(debuggee_executable);
//
//    string commandSequence = NextSequence();
//    ss << commandSequence << "-file-exec-and-symbols " << debuggee_executable;
//    LOG_DEBUG() << ss.str();
//    m_process->WriteLn(ss.str());
//    m_handlersMap.insert({ commandSequence, [=](const string& output) -> dap::ProtocolMessage::Ptr_t {
//                              // Process the output. Output is guranteed to be a complete reply from the debeugger
//                              if(StringUtils::StartsWith(output, "^done")) {
//                                  dap::LaunchResponse* response = new dap::LaunchResponse();
//                                  response->success = true;
//                                  response->request_seq = requestSeq;
//                                  return dap::ProtocolMessage::Ptr_t(response);
//                              } else if(StringUtils::StartsWith(output, "^error")) {
//                                  dap::LaunchResponse* response = new dap::LaunchResponse();
//                                  response->success = false;
//                                  response->message = GdbHandler::ParseErrorMessage(output);
//                                  response->request_seq = requestSeq;
//                                  LOG_ERROR() << "Sending error response:" << response->message;
//                                  return dap::ProtocolMessage::Ptr_t(response);
//                              }
//                              return nullptr;
//                          } });
}

void GdbHandler::StartDebugger(const string& debuggerExecutable, const string& wd)
{
    m_process = dap::ExecuteProcess(debuggerExecutable + " -i=mi", wd);
    if(!m_process) {
        throw dap::Exception("Failed to start debugger process: " + debuggerExecutable);
    }
}

string GdbHandler::NextSequence()
{
    stringstream ss;
    ss << setw(6) << setfill('0') << (++m_commandCounter);
    return ss.str();
}

dap::ProtocolMessage::Ptr_t GdbHandler::OnDebuggerStdout(const string& message)
{
    if(!message.empty()) {
        m_stdout += message;
    }
    return OnOutput(m_stdout);
}

dap::ProtocolMessage::Ptr_t GdbHandler::OnDebuggerStderr(const string& message)
{
    if(!message.empty()) {
        m_stderr += message;
    }
    return OnOutput(m_stderr);
}

dap::ProtocolMessage::Ptr_t GdbHandler::OnOutput(string& inbuffer)
{
    // MI is line based
    size_t where = inbuffer.find('\n');
    if(where != string::npos) {
        string buffer = inbuffer.substr(0, where);
        StringUtils::Trim(buffer);
        inbuffer.erase(0, where + 1); // skip "\n"
        LOG_DEBUG() << "Processing line input from GDB:" << buffer;
        switch(buffer[0]) {
        case '=':
        case '+':
        case '*':
        case '@':
        case '&':
        case '~': {
            // console stream output
            LOG_DEBUG() << buffer;
            return SendOutputEvent(buffer);

        } break;
        default: {
            if(buffer.length() >= 6) {
                string seq = buffer.substr(0, 6);
                buffer.erase(0, 6);
                auto iter = m_handlersMap.find(seq);
                if(iter != m_handlersMap.end()) {
                    return iter->second(buffer);
                } else {
                    return SendOutputEvent(buffer);
                }
            } else {
                return SendOutputEvent(buffer);
            }
        } break;
        }
    }
    return nullptr;
}

dap::ProtocolMessage::Ptr_t GdbHandler::SendOutputEvent(const string& buffer)
{
    dap::OutputEvent* outputEvent = new dap::OutputEvent();
    outputEvent->output = buffer;
    outputEvent->category = "console";
    return dap::ProtocolMessage::Ptr_t(outputEvent);
}
