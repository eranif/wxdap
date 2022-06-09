#include "MainFrame.hpp"
#include "dap/Log.hpp"
#include <wx/ffile.h>
#include <wx/msgdlg.h>

namespace
{
void center_line(wxStyledTextCtrl* ctrl, int line = wxNOT_FOUND)
{
    if(line == wxNOT_FOUND) {
        line = ctrl->LineFromPosition(ctrl->GetLastPosition());
    }

    int pos = ctrl->PositionFromLine(line);
    ctrl->SetCurrentPos(pos);
    ctrl->SetSelectionStart(pos);
    ctrl->SetSelectionEnd(pos);
    ctrl->SetAnchor(pos);
    ctrl->EnsureCaretVisible();
}
} // namespace

MainFrame::MainFrame(wxWindow* parent)
    : MainFrameBase(parent)
{
    wxFont code_font = wxFont(wxFontInfo(12).Family(wxFONTFAMILY_TELETYPE));
    for(int i = 0; i < wxSTC_STYLE_MAX; ++i) {
        m_stcLog->StyleSetFont(i, code_font);
        m_stcText->StyleSetFont(i, code_font);
        m_stcThreads->StyleSetFont(i, code_font);
        m_stcStack->StyleSetFont(i, code_font);
    }
}

MainFrame::~MainFrame() {}

/// Initialize the DAP client:
/// - Bind events
/// - Connect
/// - And launch our debuggee process
void MainFrame::InitializeClient()
{
    m_client.Bind(wxEVT_DAP_STOPPED_EVENT, &MainFrame::OnStopped, this);
    m_client.Bind(wxEVT_DAP_INITIALIZED_EVENT, &MainFrame::OnInitialized, this);
    m_client.Bind(wxEVT_DAP_EXITED_EVENT, &MainFrame::OnExited, this);
    m_client.Bind(wxEVT_DAP_TERMINATED_EVENT, &MainFrame::OnTerminated, this);
    m_client.Bind(wxEVT_DAP_STACKTRACE_RESPONSE, &MainFrame::OnStackTrace, this);
    m_client.Bind(wxEVT_DAP_OUTPUT_EVENT, &MainFrame::OnOutput, this);

    wxBusyCursor cursor;
    if(!m_client.Connect("tcp://127.0.0.1:12345", 10)) {
        wxMessageBox("Failed to connect to DAP server", "DAP Demo", wxICON_ERROR | wxOK | wxCENTRE);
        exit(1);
    }

    // This part is done in mode **sync**
    m_client.Initialize();
    m_client.ConfigurationDone();
    m_client.Launch({ R"(C:\Users\eran\Downloads\testclangd\Debug\testclangd.exe)" });
}

void MainFrame::OnNext(wxCommandEvent& event)
{
    wxUnusedVar(event);
    m_client.Next();
}

void MainFrame::OnStepIn(wxCommandEvent& event) {}

void MainFrame::OnStepOut(wxCommandEvent& event) {}
void MainFrame::OnConnect(wxCommandEvent& event)
{
    wxUnusedVar(event);
    InitializeClient();
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

        if(stopped_data->reason == "exception") {
            // apply breakpoints
            m_client.SetBreakpointsFile("main.cpp", { { 17, "" } });
            // Continue
            m_client.Continue();
        } else if(stopped_data->reason == "breakpoint" /* breakpoint hit */
                  || stopped_data->reason == "step" /* user called "Next", "StepIn", "StepOut" */) {
            // request the stack for the stopped thread
            m_client.GetFrames();
        }
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
            int line = stack_trace_data->stackFrames[0].line;
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
    center_line(m_stcText, line_number);
}
