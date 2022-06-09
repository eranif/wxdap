#include "Client.hpp"
#include "DAPEvent.hpp"
#include "Exception.hpp"
#include "Log.hpp"
#include "SocketClient.hpp"
#include "dap.hpp"
#include <iostream>
#include <thread>

dap::Client::Client()
{
    dap::Initialize();
    m_socket.reset(new SocketClient());
    m_shutdown.store(false);
    m_terminated.store(false);
}

dap::Client::~Client() { StopReaderThread(); }

bool dap::Client::Connect(const wxString& connection_string, int timeoutSeconds)
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
            StartReaderThread();
            return true;
        }
    }
    return false;
}

void dap::Client::Initialize()
{
    // Send initialize request
    InitializeRequest req;
    req.arguments.clientID = "dbgcli";
    m_rpc.Send(req, m_socket);
    m_handshake_state = eHandshakeState::kInProgress;
}

bool dap::Client::IsConnected() const { return m_readerThread && !m_terminated.load(); }

void dap::Client::SetBreakpointsFile(const wxString& file, const std::vector<dap::SourceBreakpoint>& lines)
{
    // Now that the initialize is done, we can call 'setBreakpoints' command
    SetBreakpointsRequest* setBreakpoints = new SetBreakpointsRequest();
    setBreakpoints->seq = GetNextSequence(); // command sequence
    setBreakpoints->arguments.breakpoints = lines;
    setBreakpoints->arguments.source.path = file;
    m_rpc.Send(ProtocolMessage::Ptr_t(setBreakpoints), m_socket);
}

void dap::Client::ConfigurationDone()
{
    ConfigurationDoneRequest* configDone = new ConfigurationDoneRequest();
    configDone->seq = GetNextSequence();
    m_rpc.Send(ProtocolMessage::Ptr_t(configDone), m_socket);
}

void dap::Client::Launch(std::vector<wxString>&& cmd, const wxString& workingDirectory, bool stopOnEntry,
                         const std::vector<wxString>& env)
{
    m_active_thread_id = wxNOT_FOUND;
    LaunchRequest* launchRequest = new LaunchRequest();
    launchRequest->seq = GetNextSequence(); // command sequence
    launchRequest->arguments.program = cmd[0];

    cmd.erase(cmd.begin());
    launchRequest->arguments.args = cmd; // the remainder are the args
    launchRequest->arguments.stopOnEntry = stopOnEntry;
    launchRequest->arguments.env = env;

    // set the working directory
    launchRequest->arguments.cwd = workingDirectory;
    if(launchRequest->arguments.cwd.empty()) {
        launchRequest->arguments.cwd = ::wxGetCwd();
    }
    m_waiting_for_stopped_on_entry = stopOnEntry;
    m_rpc.Send(ProtocolMessage::Ptr_t(launchRequest), m_socket);
}

void dap::Client::GetThreads()
{
    ThreadsRequest* threadsRequest = new ThreadsRequest();
    threadsRequest->seq = GetNextSequence();
    m_rpc.Send(ProtocolMessage::Ptr_t(threadsRequest), m_socket);
}

void dap::Client::GetScopes(int frameId)
{
    ScopesRequest* scopesRequest = new ScopesRequest();
    scopesRequest->arguments.frameId = frameId;
    scopesRequest->seq = GetNextSequence();
    m_rpc.Send(ProtocolMessage::Ptr_t(scopesRequest), m_socket);
}

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
    if(m_readerThread) {
        return;
    }

    m_readerThread = new thread(
        [this](dap::Client* sink) {
            LOG_INFO() << "Reader thread successfully started" << endl;
            while(!m_shutdown.load()) {
                try {
                    wxString content;
                    if(m_socket->SelectReadMS(10) == Socket::kSuccess && m_socket->Read(content) == Socket::kSuccess) {
                        sink->CallAfter(&dap::Client::OnDataRead, content);
                    }
                } catch(Exception& e) {
                    LOG_ERROR() << "Connection error: " << e.What() << endl;
                    m_terminated.store(true);
                    break;
                }
            }
            LOG_INFO() << "Going down" << endl;
        },
        this);
}

void dap::Client::OnDataRead(const wxString& buffer)
{
    // process the buffer
    if(buffer.empty())
        return;

    m_rpc.AppendBuffer(buffer);

    // dap::Client::StaticOnJsonRead will get called for every JSON payload that will arrive over the network
    m_rpc.ProcessBuffer(dap::Client::StaticOnJsonRead, this);
}

void dap::Client::StaticOnJsonRead(JSON json, wxObject* o)
{
    dap::Client* This = static_cast<dap::Client*>(o);
    This->OnJsonRead(json);
}

void dap::Client::OnJsonRead(JSON json)
{
    if(m_handshake_state != eHandshakeState::kCompleted) {
        // construct a message from the JSON
        ProtocolMessage::Ptr_t msg = ObjGenerator::Get().FromJSON(json);
        if(msg && msg->type == "response" && msg->As<Response>()->command == "initialize") {
            m_handshake_state = eHandshakeState::kCompleted;
            LOG_ERROR() << json.ToString() << endl;
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
            if(m_waiting_for_stopped_on_entry) {
                m_waiting_for_stopped_on_entry = false;
                SendDAPEvent(wxEVT_DAP_STOPPED_ON_ENTRY_EVENT, new dap::StoppedEvent, json);
            } else {
                SendDAPEvent(wxEVT_DAP_STOPPED_EVENT, new dap::StoppedEvent, json);
            }
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
            LOG_ERROR() << "Received JSON Event payload:" << endl;
            LOG_ERROR() << json.ToString(false) << endl;
        }
    } else if(as_response) {
        if(as_response->command == "stackTrace") {
            // received a stack trace response
            SendDAPEvent(wxEVT_DAP_STACKTRACE_RESPONSE, new dap::StackTraceResponse, json);
        } else {
            LOG_ERROR() << json.ToString(false) << endl;
        }
    } else {
        LOG_ERROR() << "Received JSON payload:" << endl;
        LOG_ERROR() << json.ToString(false) << endl;
    }
}

void dap::Client::SendDAPEvent(wxEventType type, ProtocolMessage* dap_message, JSON json)
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

void dap::Client::GetFrames(int threadId, int starting_frame, int frame_count)
{
    StackTraceRequest* req = new StackTraceRequest();
    req->seq = GetNextSequence();
    req->arguments.threadId = threadId == wxNOT_FOUND ? GetActiveThreadId() : threadId;
    req->arguments.levels = frame_count;
    req->arguments.startFrame = starting_frame;
    m_rpc.Send(ProtocolMessage::Ptr_t(req), m_socket);
}

void dap::Client::Next(int threadId)
{
    NextRequest* req = new NextRequest();
    req->seq = GetNextSequence();
    req->arguments.threadId = threadId == wxNOT_FOUND ? GetActiveThreadId() : threadId;
    m_rpc.Send(ProtocolMessage::Ptr_t(req), m_socket);
}

void dap::Client::Continue()
{
    ContinueRequest* req = new ContinueRequest();
    req->seq = GetNextSequence();
    m_rpc.Send(ProtocolMessage::Ptr_t(req), m_socket);
}

void dap::Client::Cleanup()
{
    StopReaderThread();
    m_socket.reset(new SocketClient());
    m_shutdown.store(false);
    m_terminated.store(false);
    m_rpc = {};
    m_requestSeuqnce = 0;
    m_handshake_state = eHandshakeState::kNotPerformed;
    m_active_thread_id = wxNOT_FOUND;
    m_waiting_for_stopped_on_entry = false;
}

void dap::Client::SetFunctionBreakpoints(const wxString& function)
{
    // place breakpoint based on function name
}
