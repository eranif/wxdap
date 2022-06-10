#include "MainFrame.hpp"

#include "dap/Log.hpp"

#include <vector>
#include <wx/ffile.h>
#include <wx/msgdlg.h>

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

MainFrame::MainFrame(wxWindow* parent)
    : MainFrameBase(parent)
{
    wxFont code_font = wxFont(wxFontInfo(12).Family(wxFONTFAMILY_TELETYPE));

    m_ctrls = { m_stcLog, m_stcText, m_stcThreads, m_stcStack };
    for(int i = 0; i < wxSTC_STYLE_MAX; ++i) {
        for(auto ctrl : m_ctrls) {
            ctrl->StyleSetFont(i, code_font);
        }
    }

    for(auto ctrl : m_ctrls) {
        ctrl->MarkerDefine(MARKER_NUMBER, wxSTC_MARK_ARROW, *wxGREEN, *wxGREEN);
    }
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
    if(!transport->Connect("tcp://127.0.0.1:12345", 10)) {
        wxMessageBox("Failed to connect to DAP server", "DAP Demo", wxICON_ERROR | wxOK | wxCENTRE);
        exit(1);
    }

    // construct new client with the transport
    m_client.SetTransport(transport);

    // bind the client events
    m_client.Bind(wxEVT_DAP_STOPPED_EVENT, &MainFrame::OnStopped, this);
    m_client.Bind(wxEVT_DAP_STOPPED_ON_ENTRY_EVENT, &MainFrame::OnStoppedOnFirstEntry, this);
    m_client.Bind(wxEVT_DAP_INITIALIZED_EVENT, &MainFrame::OnInitialized, this);
    m_client.Bind(wxEVT_DAP_EXITED_EVENT, &MainFrame::OnExited, this);
    m_client.Bind(wxEVT_DAP_TERMINATED_EVENT, &MainFrame::OnTerminated, this);
    m_client.Bind(wxEVT_DAP_STACKTRACE_RESPONSE, &MainFrame::OnStackTrace, this);
    m_client.Bind(wxEVT_DAP_OUTPUT_EVENT, &MainFrame::OnOutput, this);

    // This part is done in mode **sync**
    m_client.Initialize();
    m_client.ConfigurationDone();
    m_client.Launch({ R"(C:\Users\eran\Downloads\testclangd\Debug\testclangd.exe)" }, wxEmptyString, true);
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

void MainFrame::OnConnect(wxCommandEvent& event)
{
    wxUnusedVar(event);
    InitializeClient();
}

/// DAP server stopped due to `stopOnEntry` true when called Launch()
void MainFrame::OnStoppedOnFirstEntry(DAPEvent& event)
{
    dap::StoppedEvent* stopped_data = event.GetDapEvent()->As<dap::StoppedEvent>();
    if(stopped_data) {
        AddLog("Stopped on first entry!");
        AddLog("Placing breakpoint at main...");
        // Apply breakpoints and continue
        m_client.SetFunctionBreakpoints({ { "main" } });
        m_client.Continue();
    }
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
void MainFrame::OnStackTrace(DAPEvent& event)
{
    dap::StackTraceResponse* stack_trace_data = event.GetDapResponse()->As<dap::StackTraceResponse>();
    if(stack_trace_data) {
        m_stcStack->ClearAll();
        AddLog("Received stack trace event");
        if(!stack_trace_data->stackFrames.empty()) {
            wxString file = stack_trace_data->stackFrames[0].source.path;
            int line = stack_trace_data->stackFrames[0].line - 1; // 0 based lines
            LoadFile(file, line);
        }

        for(const auto& stack : stack_trace_data->stackFrames) {
            m_stcStack->AppendText(wxString() << stack.id << "," << stack.name << "," << stack.source.path << ","
                                              << stack.line << "\n");
        }
    }
}

/// DAP server responded to our `initialize` request
void MainFrame::OnInitialized(DAPEvent& event)
{
    // got initialized event
    AddLog(wxString() << "Received" << event.GetDapEvent()->event << "event");
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
    m_current_file_loaded.Clear();
}

void MainFrame::OnOutput(DAPEvent& event)
{
    dap::OutputEvent* output_data = event.GetDapEvent()->As<dap::OutputEvent>();
    if(output_data) {
        AddLog(wxString() << output_data->category << ":" << output_data->output);
    }
}

void MainFrame::AddLog(const wxString& log)
{
    m_stcLog->AppendText(log + "\n");
    center_line(m_stcLog);
}

void MainFrame::LoadFile(const wxString& filepath, int line_number)
{
    wxFileName fp(filepath);
    if(fp != m_current_file_loaded) {
        // the is already loaded
        wxString file_to_load = fp.GetFullPath();
        AddLog(wxString() << "Loading file.." << file_to_load);
        wxFFile f(file_to_load, "rb");
        if(!f.IsOpened()) {
            AddLog(wxString() << "Failed to open file: " << file_to_load);
            return;
        }
        m_current_file_loaded = fp;
        m_stcText->LoadFile(m_current_file_loaded.GetFullPath());
    }
    center_line(m_stcText, line_number, true);
}

void MainFrame::OnNextUI(wxUpdateUIEvent& event) { event.Enable(m_client.IsConnected() && m_client.CanInteract()); }
void MainFrame::OnStepInUI(wxUpdateUIEvent& event) { event.Enable(m_client.IsConnected() && m_client.CanInteract()); }
void MainFrame::OnStepOutUI(wxUpdateUIEvent& event) { event.Enable(m_client.IsConnected() && m_client.CanInteract()); }
void MainFrame::OnConnectUI(wxUpdateUIEvent& event) { event.Enable(!m_client.IsConnected()); }
