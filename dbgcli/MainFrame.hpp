#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

#include "UI.hpp"
#include "dap/Client.hpp"
#include "dap/Process.hpp"

#include <vector>
#include <wx/any.h>
#include <wx/filename.h>

class MainFrame : public MainFrameBase
{
    dap::Client m_client;
    wxString m_ExecutableFileName;
    dap::Source m_current_source;
    std::vector<wxStyledTextCtrl*> m_ctrls;
    dap::Process* m_process = nullptr;
    int m_frame_id = wxNOT_FOUND;

public:
    MainFrame(wxWindow* parent, wxString executableFileName);
    virtual ~MainFrame();

protected:
    void OnEval(wxCommandEvent& event) override;
    void OnEvalUI(wxUpdateUIEvent& event) override;
    void InitializeClient();
    void AddLog(const wxString& log);
    void LoadFile(const dap::Source& sourceId, int line_number);

protected:
    void OnSetBreakpoint(wxCommandEvent& event) override;
    void OnSetBreakpointUI(wxUpdateUIEvent& event) override;
    void OnPause(wxCommandEvent& event) override;
    void OnPauseUI(wxUpdateUIEvent& event) override;
    void OnConnectUI(wxUpdateUIEvent& event) override;
    void OnNextUI(wxUpdateUIEvent& event) override;
    void OnStepInUI(wxUpdateUIEvent& event) override;
    void OnStepOutUI(wxUpdateUIEvent& event) override;
    void OnConnect(wxCommandEvent& event) override;
    void OnNext(wxCommandEvent& event) override;
    void OnStepIn(wxCommandEvent& event) override;
    void OnStepOut(wxCommandEvent& event) override;
    void OnContinue(wxCommandEvent& event) override;
    void OnContinueUI(wxUpdateUIEvent& event) override;

    void OnDebugFileNameChanged(wxFileDirPickerEvent& evt);

    /// Dap events
    void OnStopped(DAPEvent& event);
    void OnStackTrace(DAPEvent& event);
    void OnScopes(DAPEvent& event);
    void OnVariables(DAPEvent& event);
    void OnInitializedEvent(DAPEvent& event);
    void OnInitializeResponse(DAPEvent& event);
    void OnExited(DAPEvent& event);
    void OnTerminated(DAPEvent& event);
    void OnOutput(DAPEvent& event);
    void OnBreakpointLocations(DAPEvent& event);
    void OnConnectionError(DAPEvent& event);
    void OnBreakpointSet(DAPEvent& event);
    void OnLaunchResponse(DAPEvent& event);
    void OnRunInTerminalRequest(DAPEvent& event);
    void OnDapLog(DAPEvent& event);
};
#endif // MAINFRAME_HPP
