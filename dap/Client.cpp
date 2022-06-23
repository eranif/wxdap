#include "Client.hpp"

#include "DAPEvent.hpp"
#include "Exception.hpp"
#include "Log.hpp"
#include "SocketClient.hpp"
#include "dap.hpp"

#include <iostream>
#include <thread>
#include <wx/msgdlg.h>

///----------------------------------------------
/// Socket
///----------------------------------------------
namespace
{
template <typename RequestType>
RequestType create_dap_request(dap::Client* client)
{
    RequestType req;
    req.seq = client->GetNextSequence();
    return req;
}
} // namespace

dap::SocketTransport::SocketTransport() { m_socket = new SocketClient(); }

dap::SocketTransport::~SocketTransport()
{
    // delete the socket
    wxDELETE(m_socket);
}

bool dap::SocketTransport::Connect(const wxString& connection_string, int timeoutSeconds)
{
    long loops = timeoutSeconds;
#ifndef _WIN32
    loops *= 1000;
#endif
    while(loops) {
        if(!m_socket->As<SocketClient>()->Connect(connection_string)) {
            this_thread::sleep_for(chrono::milliseconds(1));
            --loops;
        } else {
            LOG_INFO() << "Successfully connected to DAP server" << endl;
            return true;
        }
    }
    return false;
}

bool dap::SocketTransport::Read(wxString& buffer, int msTimeout)
{
    try {
        buffer.clear();
        if(m_socket->SelectReadMS(msTimeout) == Socket::kTimeout) {
            buffer.clear();
            return true;
        } else {
            // success
            if(m_socket->Read(buffer) != Socket::kSuccess) {
                return false;
            }
            return true;
        }
    } catch(Exception& e) {
        LOG_ERROR() << e.What() << endl;
        return false;
    }
    return false;
}

size_t dap::SocketTransport::Send(const wxString& buffer)
{
    try {
        m_socket->Send(buffer);
        return buffer.length();
    } catch(Exception& e) {
        return 0;
    }
}

///----------------------------------------------
// Client
///----------------------------------------------
dap::Client::Client()
{
    dap::Initialize();
    m_shutdown.store(false);
    m_terminated.store(false);
}

dap::Client::~Client()
{
    StopReaderThread();
    wxDELETE(m_transport);
}

void dap::Client::SetTransport(dap::Transport* transport)
{
    Reset();
    wxDELETE(m_transport);
    m_transport = transport;
    StartReaderThread();
}

bool dap::Client::IsConnected() const { return m_readerThread && !m_terminated.load(); }

void dap::Client::StopReaderThread()
{
    if(!m_readerThread) {
        return;
    }
    m_shutdown.store(true);
    m_readerThread->join();
    wxDELETE(m_readerThread);
}

void dap::Client::StartReaderThread()
{
    if(m_readerThread || !m_transport) {
        return;
    }

    m_readerThread = new thread(
        [this](dap::Client* sink) {
            LOG_INFO() << "Reader thread successfully started" << endl;
            while(!m_shutdown.load()) {
                wxString content;
                bool success = m_transport->Read(content, 10);
                if(success && !content.empty()) {
                    sink->CallAfter(&dap::Client::OnDataRead, content);
                } else if(!success) {
                    m_terminated.store(true);
                    sink->CallAfter(&dap::Client::OnConnectionError);
                    break;
                }
            }
        },
        this);
}

void dap::Client::OnConnectionError()
{
    DAPEvent event{ wxEVT_DAP_LOST_CONNECTION };
    event.SetEventObject(this);
    ProcessEvent(event);
    Reset();
}

void dap::Client::OnDataRead(const wxString& buffer)
{
    // process the buffer
    if(buffer.empty())
        return;

    m_rpc.AppendBuffer(buffer);

    // dap::Client::StaticOnDataRead will get called for every Json payload that will arrive over the network
    m_rpc.ProcessBuffer(dap::Client::StaticOnDataRead, this);
}

void dap::Client::StaticOnDataRead(Json json, wxObject* o)
{
    dap::Client* This = static_cast<dap::Client*>(o);
    This->OnMessage(json);
}

#define ENABLE_FEATURE(FeatureName)         \
    if(body[#FeatureName].GetBool(false)) { \
        m_features |= FeatureName;          \
    }

void dap::Client::OnMessage(Json json)
{
    if(m_handshake_state != eHandshakeState::kCompleted) {
        // construct a message from the Json
        ProtocolMessage::Ptr_t msg = ObjGenerator::Get().FromJSON(json);
        if(msg && msg->type == "response" && msg->As<Response>()->command == "initialize") {
            m_handshake_state = eHandshakeState::kCompleted;
            // turn the feature bits
            auto body = json["body"];
            ENABLE_FEATURE(supportsConfigurationDoneRequest);
            ENABLE_FEATURE(supportsFunctionBreakpoints);
            ENABLE_FEATURE(supportsConditionalBreakpoints);
            ENABLE_FEATURE(supportsHitConditionalBreakpoints);
            ENABLE_FEATURE(supportsEvaluateForHovers);
            ENABLE_FEATURE(supportsStepBack);
            ENABLE_FEATURE(supportsSetVariable);
            ENABLE_FEATURE(supportsRestartFrame);
            ENABLE_FEATURE(supportsGotoTargetsRequest);
            ENABLE_FEATURE(supportsStepInTargetsRequest);
            ENABLE_FEATURE(supportsCompletionsRequest);
            ENABLE_FEATURE(supportsModulesRequest);
            ENABLE_FEATURE(supportsRestartRequest);
            ENABLE_FEATURE(supportsExceptionOptions);
            ENABLE_FEATURE(supportsValueFormattingOptions);
            ENABLE_FEATURE(supportsExceptionInfoRequest);
            ENABLE_FEATURE(supportTerminateDebuggee);
            ENABLE_FEATURE(supportsDelayedStackTraceLoading);
            ENABLE_FEATURE(supportsLoadedSourcesRequest);
            ENABLE_FEATURE(supportsProgressReporting);
            ENABLE_FEATURE(supportsRunInTerminalRequest);
            ENABLE_FEATURE(supportsBreakpointLocationsRequest);
            SendDAPEvent(wxEVT_DAP_INITIALIZE_RESPONSE, new dap::InitializeResponse, json);
        }
        return;
    }

    // Other messages, convert the DAP message into wxEvent and fire it here
    auto msg = dap::ObjGenerator::Get().FromJSON(json);

    auto as_event = msg->AsEvent();
    auto as_response = msg->AsResponse();
    if(as_event) {
        // received an event
        if(as_event->event == "stopped") {
            m_can_interact = true;
            SendDAPEvent(wxEVT_DAP_STOPPED_EVENT, new dap::StoppedEvent, json);
        } else if(as_event->event == "process") {
            SendDAPEvent(wxEVT_DAP_PROCESS_EVENT, new dap::ProcessEvent, json);
        } else if(as_event->event == "exited") {
            SendDAPEvent(wxEVT_DAP_EXITED_EVENT, new dap::ExitedEvent, json);
        } else if(as_event->event == "terminated") {
            SendDAPEvent(wxEVT_DAP_TERMINATED_EVENT, new dap::TerminatedEvent, json);
        } else if(as_event->event == "initialized") {
            SendDAPEvent(wxEVT_DAP_INITIALIZED_EVENT, new dap::InitializedEvent, json);
        } else if(as_event->event == "output") {
            SendDAPEvent(wxEVT_DAP_OUTPUT_EVENT, new dap::OutputEvent, json);
        } else {
            LOG_ERROR() << "Received Json Event payload:" << endl;
            LOG_ERROR() << json.ToString(false) << endl;
        }
    } else if(as_response) {
        if(as_response->command == "stackTrace") {
            // received a stack trace response
            auto response = new dap::StackTraceResponse;
            response->From(json);
            response->threadId = m_get_frames_queue.front();
            m_get_frames_queue.erase(m_get_frames_queue.begin());

            SendDAPEvent(wxEVT_DAP_STACKTRACE_RESPONSE, response, json);

        } else if(as_response->command == "scopes") {
            // Scopes response.
            // Usually contains top level items to display in the tree view of the variables
            // example:
            //
            // > Locals
            // > Registers
            // > Globals
            //
            SendDAPEvent(wxEVT_DAP_SCOPES_RESPONSE, new dap::ScopesResponse, json);
        } else if(as_response->command == "variables") {
            SendDAPEvent(wxEVT_DAP_VARIABLES_RESPONSE, new dap::VariablesResponse, json);

        } else if(as_response->command == "stepIn" || as_response->command == "stepOut" ||
                  as_response->command == "next" || as_response->command == "continue") {
            // the above responses indicate that the debugger accepted the corresponding command and can not be
            // interacted for now
            m_can_interact = false;

        } else if(as_response->command == "breakpointLocations") {
            // special handling for breakpoint locations response:
            // we would also like to pass the origin source file that was passed as part of the
            // request
            auto ptr = new dap::BreakpointLocationsResponse;
            if(m_requestIdToFilepath.count(as_response->request_seq)) {
                ptr->filepath = m_requestIdToFilepath[as_response->request_seq];
                m_requestIdToFilepath.erase(as_response->request_seq);
            }
            SendDAPEvent(wxEVT_DAP_BREAKPOINT_LOCATIONS_RESPONSE, ptr, json);

        } else if(as_response->command == "setFunctionBreakpoints") {
            SendDAPEvent(wxEVT_DAP_SET_FUNCTION_BREAKPOINT_RESPONSE, new dap::SetFunctionBreakpointsResponse, json);

        } else if(as_response->command == "setBreakpoints") {
            SendDAPEvent(wxEVT_DAP_SET_SOURCE_BREAKPOINT_RESPONSE, new dap::SetBreakpointsResponse, json);

        } else if(as_response->command == "configurationDone") {
            SendDAPEvent(wxEVT_DAP_CONFIGURARIONE_DONE_RESPONSE, new dap::ConfigurationDoneResponse, json);
        } else if(as_response->command == "launch") {
            SendDAPEvent(wxEVT_DAP_LAUNCH_RESPONSE, new dap::LaunchResponse, json);
        } else if(as_response->command == "threads") {
            SendDAPEvent(wxEVT_DAP_THREADS_RESPONSE, new dap::ThreadsResponse, json);
        }
    } else {
        // LOG_ERROR() << "Received Json payload:" << endl;
        // LOG_ERROR() << json.ToString(false) << endl;
    }
}

void dap::Client::SendDAPEvent(wxEventType type, ProtocolMessage* dap_message, Json json)
{
    std::shared_ptr<dap::Any> ptr{ dap_message };
    ptr->From(json);
    if(type == wxEVT_DAP_STOPPED_EVENT) {
        // keep track of the current active thread ID
        m_active_thread_id = ptr->As<StoppedEvent>()->threadId;
    }

    DAPEvent event(type);
    event.SetAnyObject(ptr);
    event.SetEventObject(this);
    ProcessEvent(event);
}

void dap::Client::Reset()
{
    StopReaderThread();
    wxDELETE(m_transport);
    m_shutdown.store(false);
    m_terminated.store(false);
    m_rpc = {};
    m_requestSeuqnce = 0;
    m_handshake_state = eHandshakeState::kNotPerformed;
    m_active_thread_id = wxNOT_FOUND;
    m_can_interact = false;
    m_requestIdToFilepath.clear();
    m_features = 0;
    m_get_frames_queue.clear();
}

/// API
void dap::Client::Initialize()
{
    // Send initialize request
    InitializeRequest req = create_dap_request<InitializeRequest>(this);
    req.arguments.clientID = "dbgcli";
    SendRequest(req);
    m_handshake_state = eHandshakeState::kInProgress;
}

void dap::Client::SetBreakpointsFile(const wxString& file, const std::vector<dap::SourceBreakpoint>& lines)
{
    // Now that the initialize is done, we can call 'setBreakpoints' command
    SetBreakpointsRequest req = create_dap_request<SetBreakpointsRequest>(this) =
        create_dap_request<SetBreakpointsRequest>(this);
    req.arguments.breakpoints = lines;
    req.arguments.source.path = file;
    SendRequest(req);
}

void dap::Client::ConfigurationDone()
{
    ConfigurationDoneRequest req = create_dap_request<ConfigurationDoneRequest>(this) =
        create_dap_request<ConfigurationDoneRequest>(this);
    SendRequest(req);
}

void dap::Client::Launch(std::vector<wxString>&& cmd, const wxString& workingDirectory)
{
    m_active_thread_id = wxNOT_FOUND;
    LaunchRequest req = create_dap_request<LaunchRequest>(this) = create_dap_request<LaunchRequest>(this);
    req.arguments.program = cmd[0];

    cmd.erase(cmd.begin());
    req.arguments.args = cmd; // the remainder are the args

    // set the working directory
    req.arguments.cwd = workingDirectory;

    SendRequest(req);
}

void dap::Client::GetThreads()
{
    ThreadsRequest req = create_dap_request<ThreadsRequest>(this) = create_dap_request<ThreadsRequest>(this);
    SendRequest(req);
}

void dap::Client::GetScopes(int frameId)
{
    ScopesRequest req = create_dap_request<ScopesRequest>(this) = create_dap_request<ScopesRequest>(this);
    req.arguments.frameId = frameId;
    SendRequest(req);
}

void dap::Client::GetFrames(int threadId, int starting_frame, int frame_count)
{
    StackTraceRequest req = create_dap_request<StackTraceRequest>(this);
    req.arguments.threadId = threadId == wxNOT_FOUND ? GetActiveThreadId() : threadId;
    req.arguments.levels = frame_count;
    req.arguments.startFrame = starting_frame;

    m_get_frames_queue.push_back(req.arguments.threadId);
    SendRequest(req);
}

void dap::Client::Next(int threadId)
{
    NextRequest req = create_dap_request<NextRequest>(this);
    req.arguments.threadId = threadId == wxNOT_FOUND ? GetActiveThreadId() : threadId;
    SendRequest(req);
}

void dap::Client::Continue()
{
    ContinueRequest req = create_dap_request<ContinueRequest>(this);
    SendRequest(req);
}

void dap::Client::SetFunctionBreakpoints(const std::vector<dap::FunctionBreakpoint>& breakpoints)
{
    // place breakpoint based on function name
    SetFunctionBreakpointsRequest req = create_dap_request<SetFunctionBreakpointsRequest>(this);
    req.arguments.breakpoints = breakpoints;
    SendRequest(req);
}

void dap::Client::StepIn(int threadId)
{
    StepInRequest req = create_dap_request<StepInRequest>(this);
    req.arguments.threadId = threadId == wxNOT_FOUND ? GetActiveThreadId() : threadId;
    SendRequest(req);
}

void dap::Client::StepOut(int threadId)
{
    StepOutRequest req = create_dap_request<StepOutRequest>(this);
    req.arguments.threadId = threadId == wxNOT_FOUND ? GetActiveThreadId() : threadId;
    SendRequest(req);
}

void dap::Client::GetChildrenVariables(int variablesReference, size_t count, const wxString& format)
{
    VariablesRequest req = create_dap_request<VariablesRequest>(this);
    req.arguments.variablesReference = variablesReference;
    req.arguments.count = count;
    SendRequest(req);
}

void dap::Client::Pause(int threadId)
{
    PauseRequest req = create_dap_request<PauseRequest>(this);
    req.arguments.threadId = threadId == wxNOT_FOUND ? GetActiveThreadId() : threadId;
    SendRequest(req);
}

void dap::Client::BreakpointLocations(const wxString& filepath, int start_line, int end_line)
{
    if(!IsSupported(supportsBreakpointLocationsRequest)) {
        return;
    }

    BreakpointLocationsRequest req = create_dap_request<BreakpointLocationsRequest>(this);
    req.arguments.source.path = filepath;
    req.arguments.line = start_line;
    req.arguments.endLine = end_line;
    SendRequest(req);
    m_requestIdToFilepath.insert({ req.seq, filepath });
}

bool dap::Client::SendRequest(dap::ProtocolMessage& request)
{
    try {
        m_rpc.Send(request, m_transport);
    } catch(Exception& e) {
        // an error occured
        OnConnectionError();
        return false;
    }
    return true;
}
