#include "Client.hpp"

#include "DAPEvent.hpp"
#include "Exception.hpp"
#include "Log.hpp"
#include "SocketClient.hpp"
#include "dap.hpp"

#include <iostream>
#include <thread>
#include <wx/ffile.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>

///----------------------------------------------
/// Socket
///----------------------------------------------

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

dap::Client::~Client() { Reset(); }

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

dap::Request* dap::Client::GetOriginatingRequest(dap::Response* response)
{
    if(!response || (m_in_flight_requests.count(response->request_seq) == 0)) {
        return nullptr;
    }
    auto req = m_in_flight_requests.find(response->request_seq)->second;
    m_in_flight_requests.erase(response->request_seq);
    return req;
}

#define ENABLE_FEATURE(FeatureName)         \
    if(body[#FeatureName].GetBool(false)) { \
        m_features |= FeatureName;          \
    }

void dap::Client::OnMessage(Json json)
{
    if(m_wants_log_events) {
        DAPEvent log_event{ wxEVT_DAP_LOG_EVENT };
        log_event.SetString("<-- " + json.ToString(false));
        ProcessEvent(log_event);
    }

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
            SendDAPEvent(wxEVT_DAP_INITIALIZE_RESPONSE, new dap::InitializeResponse, json, nullptr);
        }
        return;
    }

    // Other messages, convert the DAP message into wxEvent and fire it here
    auto msg = dap::ObjGenerator::Get().FromJSON(json);
    if(!msg) {
        // unsupported event type
        return;
    }

    auto as_event = msg->AsEvent();
    auto as_response = msg->AsResponse();
    auto as_request = msg->AsRequest();
    if(as_event) {
        // received an event
        if(as_event->event == "stopped") {
            m_can_interact = true;
            SendDAPEvent(wxEVT_DAP_STOPPED_EVENT, new dap::StoppedEvent, json, nullptr);
        } else if(as_event->event == "process") {
            SendDAPEvent(wxEVT_DAP_PROCESS_EVENT, new dap::ProcessEvent, json, nullptr);
        } else if(as_event->event == "exited") {
            SendDAPEvent(wxEVT_DAP_EXITED_EVENT, new dap::ExitedEvent, json, nullptr);
        } else if(as_event->event == "terminated") {
            SendDAPEvent(wxEVT_DAP_TERMINATED_EVENT, new dap::TerminatedEvent, json, nullptr);
        } else if(as_event->event == "initialized") {
            SendDAPEvent(wxEVT_DAP_INITIALIZED_EVENT, new dap::InitializedEvent, json, nullptr);
        } else if(as_event->event == "output") {
            SendDAPEvent(wxEVT_DAP_OUTPUT_EVENT, new dap::OutputEvent, json, nullptr);
        } else if(as_event->event == "breakpoint") {
            SendDAPEvent(wxEVT_DAP_BREAKPOINT_EVENT, new dap::BreakpointEvent, json, nullptr);
        } else if(as_event->event == "continued") {
            m_can_interact = false;
            SendDAPEvent(wxEVT_DAP_CONTINUED_EVENT, new dap::ContinuedEvent, json, nullptr);
        } else if(as_event->event == "module") {
            SendDAPEvent(wxEVT_DAP_MODULE_EVENT, new dap::ModuleEvent, json, nullptr);
        } else {
            // TODO implement here the rest of the event
        }
    } else if(as_response) {
        if(as_response->command == "stackTrace") {
            // received a stack trace response
            auto response = new dap::StackTraceResponse;
            if(!m_get_frames_queue.empty()) {
                response->refId = m_get_frames_queue.front();
                m_get_frames_queue.erase(m_get_frames_queue.begin());
            }
            SendDAPEvent(wxEVT_DAP_STACKTRACE_RESPONSE, response, json, GetOriginatingRequest(as_response));

        } else if(as_response->command == "scopes") {
            auto response = new dap::ScopesResponse;
            if(!m_get_scopes_queue.empty()) {
                response->refId = m_get_scopes_queue.front();
                m_get_scopes_queue.erase(m_get_scopes_queue.begin());
            }
            SendDAPEvent(wxEVT_DAP_SCOPES_RESPONSE, response, json, GetOriginatingRequest(as_response));
        } else if(as_response->command == "variables") {
            auto response = new dap::VariablesResponse;
            if(!m_get_variables_queue.empty()) {
                response->refId = m_get_variables_queue.front().first;
                response->context = m_get_variables_queue.front().second;
                m_get_variables_queue.erase(m_get_variables_queue.begin());
            }

            SendDAPEvent(wxEVT_DAP_VARIABLES_RESPONSE, response, json, GetOriginatingRequest(as_response));

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
            SendDAPEvent(wxEVT_DAP_BREAKPOINT_LOCATIONS_RESPONSE, ptr, json, GetOriginatingRequest(as_response));

        } else if(as_response->command == "setFunctionBreakpoints") {
            SendDAPEvent(wxEVT_DAP_SET_FUNCTION_BREAKPOINT_RESPONSE, new dap::SetFunctionBreakpointsResponse, json,
                         GetOriginatingRequest(as_response));

        } else if(as_response->command == "setBreakpoints") {
            auto ptr = new dap::SetBreakpointsResponse;
            if(!m_source_breakpoints_queue.empty()) {
                ptr->originSource = m_source_breakpoints_queue.front();
                m_source_breakpoints_queue.erase(m_source_breakpoints_queue.begin());
            }
            SendDAPEvent(wxEVT_DAP_SET_SOURCE_BREAKPOINT_RESPONSE, ptr, json, GetOriginatingRequest(as_response));

        } else if(as_response->command == "configurationDone") {
            SendDAPEvent(wxEVT_DAP_CONFIGURARIONE_DONE_RESPONSE, new dap::ConfigurationDoneResponse, json,
                         GetOriginatingRequest(as_response));
        } else if(as_response->command == "launch") {
            SendDAPEvent(wxEVT_DAP_LAUNCH_RESPONSE, new dap::LaunchResponse, json, GetOriginatingRequest(as_response));
        } else if(as_response->command == "threads") {
            SendDAPEvent(wxEVT_DAP_THREADS_RESPONSE, new dap::ThreadsResponse, json,
                         GetOriginatingRequest(as_response));
        } else if(as_response->command == "source") {
            HandleSourceResponse(json);
        } else if(as_response->command == "evaluate") {
            HandleEvaluateResponse(json);
        }
    } else if(as_request) {
        // reverse requests: request arriving from the dap server to the IDE
        if(as_request->command == "runInTerminal") {
            SendDAPEvent(wxEVT_DAP_RUN_IN_TERMINAL_REQUEST, new dap::RunInTerminalRequest, json, nullptr);
        }
    }
}

void dap::Client::HandleEvaluateResponse(Json json)
{
    if(m_evaluate_queue.empty()) {
        // something bad happened..
        return;
    }

    EvaluateResponse response;
    response.From(json);

    auto callback = std::move(m_evaluate_queue.front());
    m_evaluate_queue.erase(m_evaluate_queue.begin());
    callback(response.success, response.result, response.type, response.variablesReference);
}

void dap::Client::HandleSourceResponse(Json json)
{
    if(m_load_sources_queue.empty()) {
        // something bad happened..
        return;
    }

    SourceResponse response;
    response.From(json);

    auto callback = std::move(m_load_sources_queue.front());
    m_load_sources_queue.erase(m_load_sources_queue.begin());
    callback(response.success, response.content, response.mimeType);
}

void dap::Client::SendDAPEvent(wxEventType type, ProtocolMessage* dap_message, Json json, Request* req)
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
    if(req) {
        std::shared_ptr<dap::Request> request{ req };
        event.SetOriginatingReuqest(request);
    }
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
    m_load_sources_queue.clear();
    m_get_frames_queue.clear();
    m_get_scopes_queue.clear();
    m_get_variables_queue.clear();
    m_source_breakpoints_queue.clear();
    m_evaluate_queue.clear();
    for(auto& vt : m_in_flight_requests) {
        wxDELETE(vt.second);
    }
    m_in_flight_requests.clear();
}

/// API
void dap::Client::Initialize(const dap::InitializeRequestArguments* initArgs)
{
    // Send initialize request
    auto req = MakeRequest<InitializeRequest>();
    if(initArgs) {
        req->arguments = *initArgs;

    } else {
        // use the defaults
        req->arguments.clientID = "wxdap";
        req->arguments.clientName = "wxdap";
    }
    SendRequest(req);
    m_handshake_state = eHandshakeState::kInProgress;
}

void dap::Client::SetBreakpointsFile(const wxString& file, const std::vector<dap::SourceBreakpoint>& lines)
{
    // Now that the initialize is done, we can call 'setBreakpoints' command
    auto req = MakeRequest<SetBreakpointsRequest>();
    req->arguments.breakpoints = lines;
    req->arguments.source.path = file;
    req->arguments.source.name = wxFileName(file).GetFullName();

    // keep the originating source file
    m_source_breakpoints_queue.push_back(file);
    SendRequest(req);
}

void dap::Client::ConfigurationDone()
{
    auto req = MakeRequest<ConfigurationDoneRequest>();
    SendRequest(req);
}

void dap::Client::Launch(std::vector<wxString>&& cmd, const wxString& workingDirectory, const dap::Environment& env)
{
    m_active_thread_id = wxNOT_FOUND;
    auto req = MakeRequest<LaunchRequest>();
    req->arguments.program = cmd[0];

    cmd.erase(cmd.begin());
    req->arguments.args = cmd; // the remainder are the args

    // set the working directory & env vars
    req->arguments.cwd = workingDirectory;
    req->arguments.env = env;

    SendRequest(req);
}

void dap::Client::GetThreads()
{
    auto req = MakeRequest<ThreadsRequest>();
    SendRequest(req);
}

void dap::Client::GetScopes(int frameId)
{
    auto req = MakeRequest<ScopesRequest>();
    req->arguments.frameId = frameId;
    m_get_scopes_queue.push_back(frameId);
    SendRequest(req);
}

void dap::Client::GetFrames(int threadId, int starting_frame, int frame_count)
{
    auto req = MakeRequest<StackTraceRequest>();
    req->arguments.threadId = threadId == wxNOT_FOUND ? GetActiveThreadId() : threadId;
    req->arguments.levels = frame_count;
    req->arguments.startFrame = starting_frame;

    m_get_frames_queue.push_back(req->arguments.threadId);
    SendRequest(req);
}

void dap::Client::Next(int threadId, bool singleThread, SteppingGranularity granularity)
{
    auto req = MakeRequest<NextRequest>();
    req->arguments.threadId = threadId == wxNOT_FOUND ? GetActiveThreadId() : threadId;
    req->arguments.singleThread = singleThread;

    switch(granularity) {
    case SteppingGranularity::LINE:
        req->arguments.granularity = "line";
        break;
    case SteppingGranularity::STATEMENT:
        req->arguments.granularity = "statement";
        break;
    case SteppingGranularity::INSTRUCTION:
        req->arguments.granularity = "instruction";
        break;
    }
    SendRequest(req);
}

void dap::Client::Continue(int threadId, bool all_threads)
{
    auto req = MakeRequest<ContinueRequest>();
    req->arguments.threadId = threadId == wxNOT_FOUND ? GetActiveThreadId() : threadId;
    req->arguments.singleThread = !all_threads || (req->arguments.threadId == wxNOT_FOUND);
    SendRequest(req);
}

void dap::Client::SetFunctionBreakpoints(const std::vector<dap::FunctionBreakpoint>& breakpoints)
{
    // place breakpoint based on function name
    auto req = MakeRequest<SetFunctionBreakpointsRequest>();
    req->arguments.breakpoints = breakpoints;
    SendRequest(req);
}

void dap::Client::StepIn(int threadId, bool singleThread)
{
    auto req = MakeRequest<StepInRequest>();
    req->arguments.threadId = threadId == wxNOT_FOUND ? GetActiveThreadId() : threadId;
    req->arguments.singleThread = singleThread;
    SendRequest(req);
}

void dap::Client::StepOut(int threadId, bool singleThread)
{
    auto req = MakeRequest<StepOutRequest>();
    req->arguments.threadId = threadId == wxNOT_FOUND ? GetActiveThreadId() : threadId;
    req->arguments.singleThread = singleThread;
    SendRequest(req);
}

void dap::Client::GetChildrenVariables(int variablesReference, EvaluateContext context, size_t count,
                                       ValueDisplayFormat format)
{
    auto req = MakeRequest<VariablesRequest>();
    req->arguments.variablesReference = variablesReference;
    req->arguments.count = count;
    req->arguments.format.hex = (format == ValueDisplayFormat::HEX);
    m_get_variables_queue.push_back({ variablesReference, context });
    SendRequest(req);
}

void dap::Client::Pause(int threadId)
{
    auto req = MakeRequest<PauseRequest>();
    req->arguments.threadId = threadId == wxNOT_FOUND ? GetActiveThreadId() : threadId;
    SendRequest(req);
}

void dap::Client::BreakpointLocations(const wxString& filepath, int start_line, int end_line)
{
    if(!IsSupported(supportsBreakpointLocationsRequest)) {
        return;
    }

    auto req = MakeRequest<BreakpointLocationsRequest>();
    req->arguments.source.path = filepath;
    req->arguments.line = start_line;
    req->arguments.endLine = end_line;
    SendRequest(req);
    m_requestIdToFilepath.insert({ req->seq, filepath });
}

bool dap::Client::SendRequest(dap::Request* request)
{
    try {
        m_rpc.Send(static_cast<dap::ProtocolMessage&>(*request), m_transport);
        if(m_wants_log_events) {
            DAPEvent log_event{ wxEVT_DAP_LOG_EVENT };
            log_event.SetString("--> " + request->To().ToString(false));
            ProcessEvent(log_event);
        }
        m_in_flight_requests.insert({ request->seq, request });

    } catch(Exception& e) {
        // an error occured
        OnConnectionError();
        return false;
    }
    return true;
}

bool dap::Client::SendResponse(dap::Response& response)
{
    try {
        m_rpc.Send(response, m_transport);
        if(m_wants_log_events) {
            DAPEvent log_event{ wxEVT_DAP_LOG_EVENT };
            log_event.SetString("--> " + response.To().ToString(false));
            ProcessEvent(log_event);
        }
    } catch(Exception& e) {
        // an error occured
        OnConnectionError();
        return false;
    }
    return true;
}

bool dap::Client::LoadSource(const dap::Source& source, source_loaded_cb callback)
{
    if(source.sourceReference > 0) {
        // queue request
        m_load_sources_queue.emplace_back(std::move(callback));
        auto req = MakeRequest<SourceRequest>();
        req->arguments.source = source;
        req->arguments.sourceReference = source.sourceReference;
        SendRequest(req);
        return true;

    } else {
        return false;
    }
}

void dap::Client::EvaluateExpression(const wxString& expression, int frameId, EvaluateContext context,
                                     evaluate_cb callback, ValueDisplayFormat format)
{
    m_evaluate_queue.emplace_back(std::move(callback));
    auto req = MakeRequest<EvaluateRequest>();
    req->arguments.frameId = frameId;
    req->arguments.expression = expression;
    req->arguments.format.hex = (format == ValueDisplayFormat::HEX);
    switch(context) {
    case EvaluateContext::CLIPBOARD:
        req->arguments.context = "clipboard";
        break;
    case EvaluateContext::HOVER:
        req->arguments.context = "hover";
        break;
    case EvaluateContext::REPL:
        req->arguments.context = "repl";
        break;
    case EvaluateContext::VARIABLES:
        req->arguments.context = "variables";
        break;
    case EvaluateContext::WATCH:
        req->arguments.context = "watch";
        break;
    }
    SendRequest(req);
}

void dap::Client::Attach(int pid, const std::vector<wxString>& arguments)
{
    auto req = MakeRequest<AttachRequest>();
    req->arguments.arguments = arguments;
    SendRequest(req);
}
