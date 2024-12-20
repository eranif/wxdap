//////////////////////////////////////////////////////////////////////
// This file was auto-generated by codelite's wxCrafter Plugin
// wxCrafter project file: UI.wxcp
// Do not modify this file by hand!
//////////////////////////////////////////////////////////////////////

#ifndef _WXDAP_DBGCLI_UI_BASE_CLASSES_HPP
#define _WXDAP_DBGCLI_UI_BASE_CLASSES_HPP

// clang-format off
#include <wx/settings.h>
#include <wx/xrc/xmlres.h>
#include <wx/xrc/xh_bmp.h>
#include <wx/frame.h>
#include <wx/iconbndl.h>
#include <wx/artprov.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/filepicker.h>
#include <wx/splitter.h>
#include <wx/stc/stc.h>
#include <wx/notebook.h>
#include <wx/imaglist.h>
#if wxVERSION_NUMBER >= 2900
#include <wx/persist.h>
#include <wx/persist/toplevel.h>
#include <wx/persist/bookctrl.h>
#include <wx/persist/treebook.h>
#endif

#ifdef WXC_FROM_DIP
#undef WXC_FROM_DIP
#endif
#if wxVERSION_NUMBER >= 3100
#define WXC_FROM_DIP(x) wxWindow::FromDIP(x, NULL)
#else
#define WXC_FROM_DIP(x) x
#endif

// clang-format on

class MainFrameBase : public wxFrame
{
public:
    enum {
        ID_ATTACH = 10001,
    };

protected:
    wxMenuBar* m_menuBar59;
    wxMenu* m_menu60;
    wxMenuItem* m_menuItem61;
    wxMenuItem* m_menuItem63;
    wxMenuItem* m_menuItem62;
    wxToolBar* m_toolbar12;
    wxPanel* m_panel2;
    wxStaticText* m_staticTextSelectDebugFileName;
    wxFilePickerCtrl* m_filePickerSelectDebugFileName;
    wxStaticText* m_staticText64;
    wxFilePickerCtrl* m_filePickerDebugger;
    wxSplitterWindow* m_splitter4;
    wxPanel* m_splitterPageSourceFile;
    wxStyledTextCtrl* m_stcTextSourceFile;
    wxPanel* m_splitterPageDAPDebugInfo;
    wxNotebook* m_notebookDAPDebugInfo;
    wxPanel* m_panel18;
    wxStyledTextCtrl* m_stcStack;
    wxPanel* m_panel19;
    wxStyledTextCtrl* m_stcThreads;
    wxPanel* m_panel27;
    wxStyledTextCtrl* m_stcLog;
    wxPanel* m_panel30;
    wxStyledTextCtrl* m_stcScopes;

protected:
    virtual void OnClear(wxCommandEvent& event) { event.Skip(); }
    virtual void OnExit(wxCommandEvent& event) { event.Skip(); }
    virtual void OnConnect(wxCommandEvent& event) { event.Skip(); }
    virtual void OnConnectUI(wxUpdateUIEvent& event) { event.Skip(); }
    virtual void OnAttach(wxCommandEvent& event) { event.Skip(); }
    virtual void OnAttachUI(wxUpdateUIEvent& event) { event.Skip(); }
    virtual void OnNext(wxCommandEvent& event) { event.Skip(); }
    virtual void OnNextUI(wxUpdateUIEvent& event) { event.Skip(); }
    virtual void OnStepIn(wxCommandEvent& event) { event.Skip(); }
    virtual void OnStepInUI(wxUpdateUIEvent& event) { event.Skip(); }
    virtual void OnStepOut(wxCommandEvent& event) { event.Skip(); }
    virtual void OnStepOutUI(wxUpdateUIEvent& event) { event.Skip(); }
    virtual void OnContinue(wxCommandEvent& event) { event.Skip(); }
    virtual void OnContinueUI(wxUpdateUIEvent& event) { event.Skip(); }
    virtual void OnPause(wxCommandEvent& event) { event.Skip(); }
    virtual void OnPauseUI(wxUpdateUIEvent& event) { event.Skip(); }
    virtual void OnSetBreakpoint(wxCommandEvent& event) { event.Skip(); }
    virtual void OnSetBreakpointUI(wxUpdateUIEvent& event) { event.Skip(); }
    virtual void OnEval(wxCommandEvent& event) { event.Skip(); }
    virtual void OnEvalUI(wxUpdateUIEvent& event) { event.Skip(); }

public:
    wxMenuBar* GetMenuBar59() { return m_menuBar59; }
    wxToolBar* GetToolbar12() { return m_toolbar12; }
    wxStaticText* GetStaticTextSelectDebugFileName() { return m_staticTextSelectDebugFileName; }
    wxFilePickerCtrl* GetFilePickerSelectDebugFileName() { return m_filePickerSelectDebugFileName; }
    wxStaticText* GetStaticText64() { return m_staticText64; }
    wxFilePickerCtrl* GetFilePickerDebugger() { return m_filePickerDebugger; }
    wxStyledTextCtrl* GetStcTextSourceFile() { return m_stcTextSourceFile; }
    wxPanel* GetSplitterPageSourceFile() { return m_splitterPageSourceFile; }
    wxStyledTextCtrl* GetStcStack() { return m_stcStack; }
    wxPanel* GetPanel18() { return m_panel18; }
    wxStyledTextCtrl* GetStcThreads() { return m_stcThreads; }
    wxPanel* GetPanel19() { return m_panel19; }
    wxStyledTextCtrl* GetStcLog() { return m_stcLog; }
    wxPanel* GetPanel27() { return m_panel27; }
    wxStyledTextCtrl* GetStcScopes() { return m_stcScopes; }
    wxPanel* GetPanel30() { return m_panel30; }
    wxNotebook* GetNotebookDAPDebugInfo() { return m_notebookDAPDebugInfo; }
    wxPanel* GetSplitterPageDAPDebugInfo() { return m_splitterPageDAPDebugInfo; }
    wxSplitterWindow* GetSplitter4() { return m_splitter4; }
    wxPanel* GetPanel2() { return m_panel2; }
    MainFrameBase(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("DAP UI"),
                  const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(800, 600),
                  long style = wxDEFAULT_FRAME_STYLE);
    virtual ~MainFrameBase();
};

#endif
