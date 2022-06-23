#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

#include "UI.hpp"
#include "dap/Client.hpp"

#include <vector>
#include <wx/filename.h>

class MainFrame : public MainFrameBase
{
    dap::Client m_client;
    wxString m_ExecutableFileName;
    wxFileName m_current_file_loaded;
    std::vector<wxStyledTextCtrl*> m_ctrls;

public:
    MainFrame(wxWindow* parent, wxString executableFileName);
    virtual ~MainFrame();

protected:
    void InitializeClient();
    void AddLog(const wxString& log);
    void LoadFile(const wxString& filepath, int line_number);

protected:
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
    void OnExited(DAPEvent& event);
    void OnTerminated(DAPEvent& event);
    void OnOutput(DAPEvent& event);
    void OnBreakpointLocations(DAPEvent& event);
    void OnConnectionError(DAPEvent& event);
    void OnSetBreakpoint(DAPEvent& event);
    void OnLaunchResponse(DAPEvent& event);
};
#endif // MAINFRAME_HPP
