#include "MainFrame.hpp"

#include "dap/Log.hpp"
#include "dap/Process.hpp"

#include <vector>
#include <wx/ffile.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>

namespace
{
constexpr int MARKER_NUMBER = 4;

void center_line(wxStyledTextCtrl* ctrl, int line = wxNOT_FOUND, bool add_marker = false)
{
    if(line == wxNOT_FOUND) {
        line = ctrl->LineFromPosition(ctrl->GetLastPosition());
    }
    int lines_on_screen = ctrl->LinesOnScreen();
    int first_visible_line = line - lines_on_screen / 2;
    first_visible_line = wxMax(0, first_visible_line);

    int pos = ctrl->PositionFromLine(line);
    ctrl->SetCurrentPos(pos);
    ctrl->SetSelectionStart(pos);
    ctrl->SetSelectionEnd(pos);
    ctrl->SetAnchor(pos);
    ctrl->EnsureCaretVisible();
    if(add_marker) {
        ctrl->MarkerDeleteAll(MARKER_NUMBER);
        ctrl->MarkerAdd(line, MARKER_NUMBER);
    }
    ctrl->SetFirstVisibleLine(first_visible_line);
}
} // namespace

MainFrame::MainFrame(wxWindow* parent, wxString executableFileName)
    : MainFrameBase(parent)
    , m_executableFileName(executableFileName)
{

    wxFont code_font = wxFont(wxFontInfo(12).Family(wxFONTFAMILY_TELETYPE));

    m_ctrls = { m_stcLog, m_stcTextSourceFile, m_stcThreads, m_stcStack, m_stcScopes };
    for(int i = 0; i < wxSTC_STYLE_MAX; ++i) {
        for(auto ctrl : m_ctrls) {
            ctrl->StyleSetFont(i, code_font);
        }
    }

    for(auto ctrl : m_ctrls) {
        ctrl->MarkerDefine(MARKER_NUMBER, wxSTC_MARK_ARROW, *wxGREEN, *wxGREEN);
    }

    m_filePickerSelectDebugFileName->SetPath(m_executableFileName);
    m_filePickerSelectDebugFileName->Connect(m_filePickerSelectDebugFileName->GetEventType(),
                                             wxFileDirPickerEventHandler(MainFrame::OnDebugFileNameChanged), NULL,
                                             this);

    // bind the client events
    m_client.Bind(wxEVT_DAP_STOPPED_EVENT, &MainFrame::OnStopped, this);
    m_client.Bind(wxEVT_DAP_INITIALIZED_EVENT, &MainFrame::OnInitializedEvent, this);
    m_client.Bind(wxEVT_DAP_INITIALIZE_RESPONSE, &MainFrame::OnInitializeResponse, this);
    m_client.Bind(wxEVT_DAP_EXITED_EVENT, &MainFrame::OnExited, this);
    m_client.Bind(wxEVT_DAP_TERMINATED_EVENT, &MainFrame::OnTerminated, this);
    m_client.Bind(wxEVT_DAP_STACKTRACE_RESPONSE, &MainFrame::OnStackTrace, this);
    m_client.Bind(wxEVT_DAP_SCOPES_RESPONSE, &MainFrame::OnScopes, this);
    m_client.Bind(wxEVT_DAP_VARIABLES_RESPONSE, &MainFrame::OnVariables, this);
    m_client.Bind(wxEVT_DAP_OUTPUT_EVENT, &MainFrame::OnOutput, this);
    m_client.Bind(wxEVT_DAP_BREAKPOINT_LOCATIONS_RESPONSE, &MainFrame::OnBreakpointLocations, this);
    m_client.Bind(wxEVT_DAP_LOST_CONNECTION, &MainFrame::OnConnectionError, this);
    m_client.Bind(wxEVT_DAP_SET_SOURCE_BREAKPOINT_RESPONSE, &MainFrame::OnBreakpointSet, this);
    m_client.Bind(wxEVT_DAP_SET_FUNCTION_BREAKPOINT_RESPONSE, &MainFrame::OnBreakpointSet, this);
    m_client.Bind(wxEVT_DAP_LAUNCH_RESPONSE, &MainFrame::OnLaunchResponse, this);
    m_client.Bind(wxEVT_DAP_RUN_IN_TERMINAL_REQUEST, &MainFrame::OnRunInTerminalRequest, this);
    m_client.Bind(wxEVT_DAP_LOG_EVENT, &MainFrame::OnDapLog, this);
    m_client.Bind(wxEVT_DAP_MODULE_EVENT, &MainFrame::OnDapModuleEvent, this);
    m_client.SetWantsLogEvents(true); // send use log events
}

MainFrame::~MainFrame() {}

/// Initialize the DAP client:
/// - Bind events
/// - Connect
/// - And launch our debuggee process
void MainFrame::InitializeClient()
{
    // Reset the client
    m_client.Reset();

    wxBusyCursor cursor;
    // For this demo, we use socket transport. But you may choose
    // to write your own transport that implements the dap::Transport interface
    // This is useful when the user wishes to use stdin/out for communicating with
    // the dap and not over socket
    dap::SocketTransport* transport = new dap::SocketTransport();
    if(!transport->Connect("tcp://127.0.0.1:4711", 10)) {
        wxMessageBox("Failed to connect to DAP server", "DAP Demo", wxICON_ERROR | wxOK | wxCENTRE);
        exit(1);
    }

    // construct new client with the transport
    m_client.SetTransport(transport);

    // The protocol starts by us sending an initialize request
    dap::InitializeRequestArguments args;
    args.linesStartAt1 = true;

    m_frame_id = wxNOT_FOUND;
    m_current_source = {};
    m_client.Initialize(&args);
}

void MainFrame::OnNext(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_client.Next();
}

void MainFrame::OnStepIn(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_client.StepIn();
}

void MainFrame::OnStepOut(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_client.StepOut();
}

void MainFrame::OnAttach(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_attaching = true;
    InitializeClient();
}

void MainFrame::OnConnect(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_attaching = false;
    InitializeClient();
}

/// ----------------------------------
/// -- DAP EVENTS START --
/// ----------------------------------

void MainFrame::OnLaunchResponse(DAPEvent& event)
{
    // Check that the debugee was started successfully
    dap::LaunchResponse* resp = event.GetDapResponse()->As<dap::LaunchResponse>();
    if(resp && !resp->success) {
        // launch failed!
        wxMessageBox("Failed to launch debuggee: " + resp->message, "DAP",
                     wxICON_ERROR | wxOK | wxOK_DEFAULT | wxCENTRE);
        m_client.CallAfter(&dap::Client::Reset);
    }
}

/// DAP server responded to our `initialize` request
void MainFrame::OnInitializeResponse(DAPEvent& event)
{
    wxUnusedVar(event);
    if(m_attaching) {
        AddLog("Attaching to dap server");
        m_client.Attach({});

    } else {
        AddLog("Launching program: " + m_executableFileName);
        m_client.Launch({ m_executableFileName }, ::wxGetCwd());
    }
}

void MainFrame::OnInitializedEvent(DAPEvent& event)
{
    // got initialized event, place breakpoints and continue
    AddLog("Got Initialized event");
    AddLog("Placing breakpoint at main...");

    // Set breakpoint on "main"
    m_client.SetFunctionBreakpoints({ { "main" } });
    m_client.ConfigurationDone();
}

/// DAP server stopped. This can happen for multiple reasons:
/// - exception
/// - breakpoint hit
/// - step (user previously issued `Next` command)
void MainFrame::OnStopped(DAPEvent& event)
{
    // got stopped event
    dap::StoppedEvent* stopped_data = event.GetDapEvent()->As<dap::StoppedEvent>();
    if(stopped_data) {
        AddLog(wxString() << "Stopped reason:" << stopped_data->reason);
        AddLog(wxString() << "All threads stopped:" << stopped_data->allThreadsStopped);
        AddLog(wxString() << "Stopped thread ID:" << stopped_data->threadId
                          << "(active thread ID:" << m_client.GetActiveThreadId() << ")");

        m_client.GetFrames();
    }
}

/// Received a response to `GetFrames()` call
void MainFrame::OnScopes(DAPEvent& event)
{
    m_stcScopes->AppendText("-- Requesting variables for scopes --\n");
    dap::ScopesResponse* resp = event.GetDapResponse()->As<dap::ScopesResponse>();
    if(resp) {
        wxString scopes_display = "[";
        for(const auto& scope : resp->scopes) {
            scopes_display << scope.name << " id: " << scope.variablesReference << ", ";
            m_client.GetChildrenVariables(scope.variablesReference);
        }
        scopes_display.RemoveLast(2);
        scopes_display << "]\n";
        m_stcScopes->AppendText(scopes_display);
    }
    center_line(m_stcScopes);
}

void MainFrame::OnVariables(DAPEvent& event)
{
    dap::VariablesResponse* resp = event.GetDapResponse()->As<dap::VariablesResponse>();
    if(resp) {
        for(const auto& var : resp->variables) {
            wxString button = (var.variablesReference > 0 ? "> " : "  ");
            wxString value = var.value.empty() ? "\"\"" : var.value;
            button << " [ref: " << resp->refId << "] ";
            m_stcScopes->AppendText(wxString() << button << "(" << var.variablesReference << ") " << var.name << " = "
                                               << value << "\n");
        }
    }
    center_line(m_stcScopes);
}

/// Received a response to `GetFrames()` call
void MainFrame::OnStackTrace(DAPEvent& event)
{
    dap::StackTraceResponse* stack_trace_data = event.GetDapResponse()->As<dap::StackTraceResponse>();
    if(stack_trace_data) {
        m_stcStack->ClearAll();
        AddLog("Received stack trace event");
        if(!stack_trace_data->stackFrames.empty()) {
            LoadFile(stack_trace_data->stackFrames[0].source,
                     stack_trace_data->stackFrames[0].line - 1 /* 0 based lines*/);

            m_frame_id = stack_trace_data->stackFrames[0].id;

            // request the scopes for the first stack
            m_client.GetScopes(stack_trace_data->stackFrames[0].id);
        }

        for(const auto& stack : stack_trace_data->stackFrames) {
            m_stcStack->AppendText(wxString() << stack.id << "," << stack.name << "," << stack.source.path << ","
                                              << stack.line << "\n");
        }
    }
}

/// Debuggee process exited, print the exit code
void MainFrame::OnExited(DAPEvent& event)
{
    AddLog(wxString() << "Debuggee exited. Exit code:" << event.GetDapEvent()->As<dap::ExitedEvent>()->exitCode);
}

/// Debug session terminated
void MainFrame::OnTerminated(DAPEvent& event)
{
    wxUnusedVar(event);
    AddLog(wxString() << "Session terminated!");
    m_client.Reset();
    for(auto ctrl : m_ctrls) {
        ctrl->ClearAll();
    }
    m_current_source = {};
    m_frame_id = wxNOT_FOUND;
}

void MainFrame::OnOutput(DAPEvent& event)
{
    dap::OutputEvent* output_data = event.GetDapEvent()->As<dap::OutputEvent>();
    if(output_data) {
        AddLog(wxString() << output_data->category << ":" << output_data->output);
    }
}

void MainFrame::OnBreakpointLocations(DAPEvent& event)
{
    dap::BreakpointLocationsResponse* d = event.GetDapResponse()->As<dap::BreakpointLocationsResponse>();
    if(d) {
        AddLog(_("==> Breakpoints:\n"));
        for(const auto& bp : d->breakpoints) {
            AddLog(wxString() << d->filepath << ":" << bp.line);
        }
    }
}

void MainFrame::OnConnectionError(DAPEvent& event)
{
    wxUnusedVar(event);
    wxMessageBox(_("Lost connection to dap server"));
}

void MainFrame::OnBreakpointSet(DAPEvent& event)
{
    dap::SetBreakpointsResponse* resp = event.GetDapResponse()->As<dap::SetBreakpointsResponse>();
    auto request = event.GetOriginatingReuqest();
    if(!request) {
        return;
    }
    auto set_func_bp_req = request->As<dap::SetFunctionBreakpointsRequest>();
    auto set_bp_req = request->As<dap::SetBreakpointsRequest>();
    if(!set_func_bp_req && !set_bp_req)
        return;
    if(set_func_bp_req) {
        for(const auto& bp : set_func_bp_req->arguments.breakpoints) {
            AddLog("Got reply for SetFunctionBreakpoints command for function: " + bp.name);
        }

    } else if(set_bp_req) {
        AddLog("Got reply for setBreakpoint command for file: " + set_bp_req->arguments.source.path);
        for(const auto& bp : resp->breakpoints) {
            wxString message;
            message << "ID: " << bp.id << ". Verified: " << bp.verified
                    << ". File: " << (bp.source.path.empty() ? set_bp_req->arguments.source.path : bp.source.path)
                    << ". Line: " << bp.line;
            AddLog(message);
        }
    }
}

void MainFrame::OnDapLog(DAPEvent& event) { AddLog(event.GetString()); }
void MainFrame::OnDapModuleEvent(DAPEvent& event)
{
    AddLog("Got MODULE event!");
    auto event_data = event.GetDapEvent()->As<dap::ModuleEvent>();
    if(!event_data)
        return;

    wxString log_entry;
    log_entry << event_data->module.id << ": " << event_data->module.name << " " << event_data->module.symbolStatus
              << event_data->module.version << " " << event_data->module.path;
    AddLog(log_entry);
}

void MainFrame::OnRunInTerminalRequest(DAPEvent& event)
{
    AddLog("Handling `OnRunInTerminalRequest` event");
    auto request = event.GetDapRequest()->As<dap::RunInTerminalRequest>();
    if(!request) {
        return;
    }
    wxString command;
    for(const wxString& cmd : request->arguments.args) {
        command << cmd << " ";
    }

    AddLog("Starting process: " + command);
    m_process = dap::ExecuteProcess(command);
    auto response = m_client.MakeRequest<dap::RunInTerminalResponse>();
    response->request_seq = request->seq;
    if(!m_process) {
        response->success = false;
        response->processId = 0;
    } else {
        response->success = true;
        response->processId = m_process->GetProcessId();
    }
    m_client.SendResponse(*response);
    wxDELETE(response);
}

/// ----------------------------------
/// -- DAP EVENTS END --
/// ----------------------------------

void MainFrame::AddLog(const wxString& log)
{
    m_stcLog->AppendText(log + "\n");
    center_line(m_stcLog);
}

void MainFrame::LoadFile(const dap::Source& sourceId, int line_number)
{
    // easy path
    if(sourceId == m_current_source) {
        center_line(m_stcTextSourceFile, line_number, true);
        return;
    }

    if(!m_client.LoadSource(
           sourceId, [this, sourceId, line_number](bool success, const wxString& content, const wxString& mimeType) {
               if(!success) {
                   return;
               }
               m_current_source = sourceId;
               m_stcTextSourceFile->SetText(content);
               center_line(m_stcTextSourceFile, line_number, true);
           })) {
        // not a server file, load it locally
        wxFileName fp(sourceId.path);

        // the is already loaded
        wxString file_to_load = fp.GetFullPath();
        AddLog(wxString() << "Loading file.." << file_to_load);
        wxFileName fn(file_to_load);
        if(fn.FileExists()) {
            m_current_source = sourceId;
            m_stcTextSourceFile->LoadFile(fn.GetFullPath());
            center_line(m_stcTextSourceFile, line_number, true);
        }
    }
}

void MainFrame::OnNextUI(wxUpdateUIEvent& event) { event.Enable(m_client.IsConnected() && m_client.CanInteract()); }
void MainFrame::OnStepInUI(wxUpdateUIEvent& event) { event.Enable(m_client.IsConnected() && m_client.CanInteract()); }
void MainFrame::OnStepOutUI(wxUpdateUIEvent& event) { event.Enable(m_client.IsConnected() && m_client.CanInteract()); }
void MainFrame::OnConnectUI(wxUpdateUIEvent& event) { event.Enable(!m_client.IsConnected()); }
void MainFrame::OnPause(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_client.Pause();
}

void MainFrame::OnPauseUI(wxUpdateUIEvent& event) { event.Enable(m_client.IsConnected() && !m_client.CanInteract()); }
void MainFrame::OnContinueUI(wxUpdateUIEvent& event) { event.Enable(m_client.IsConnected() && m_client.CanInteract()); }
void MainFrame::OnContinue(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_client.Continue();
}

void MainFrame::OnDebugFileNameChanged(wxFileDirPickerEvent& evt)
{
    if(m_filePickerSelectDebugFileName) {
        m_executableFileName = evt.GetPath();
        m_filePickerSelectDebugFileName->SetPath(evt.GetPath());
    }
}
void MainFrame::OnSetBreakpointUI(wxUpdateUIEvent& event)
{
    event.Enable(m_client.IsConnected() && m_client.CanInteract());
}

void MainFrame::OnSetBreakpoint(wxCommandEvent& event)
{
    wxString location =
        wxGetTextFromUser("Set breakpoint", "Location",
                          wxString() << m_current_source.path << ":" << (m_stcTextSourceFile->GetCurrentLine() + 1));
    if(location.empty()) {
        return;
    }

    if(location.Contains(":")) {
        // file:line
        wxString file = location.BeforeLast(':');
        file.Trim().Trim(false);

        long line = wxNOT_FOUND;
        location.AfterLast(':').ToCLong(&line);
        m_client.SetBreakpointsFile(file, { { static_cast<int>(line), wxEmptyString } });

    } else {
        // function
        m_client.SetFunctionBreakpoints({ { location, wxEmptyString } });
    }
}
void MainFrame::OnEval(wxCommandEvent& event)
{
    wxString text = wxGetTextFromUser("Expression", "Evaluate expression", m_stcTextSourceFile->GetSelectedText());
    if(text.empty()) {
        return;
    }

    m_client.EvaluateExpression(
        text, m_frame_id, dap::EvaluateContext::HOVER,
        [this, text](bool success, const wxString& result, const wxString& type, int variablesReference) {
            wxString output;
            if(!success) {
                output << "ERROR: failed to evaluate expression: `" << text << "`";
                AddLog(output);
                return;
            }

            output << text << " = " << result << " [" << type << "]. variablesReference: " << variablesReference;
            AddLog(output);
        });
}

void MainFrame::OnEvalUI(wxUpdateUIEvent& event) { event.Enable(m_client.IsConnected() && m_client.CanInteract()); }
void MainFrame::OnAttachUI(wxUpdateUIEvent& event) { event.Enable(!m_client.IsConnected()); }
