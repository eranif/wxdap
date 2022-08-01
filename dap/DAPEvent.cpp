#include "DAPEvent.hpp"

wxDEFINE_EVENT(wxEVT_DAP_LOG_EVENT, DAPEvent);

wxDEFINE_EVENT(wxEVT_DAP_LOST_CONNECTION, DAPEvent);

wxDEFINE_EVENT(wxEVT_DAP_INITIALIZE_RESPONSE, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_STACKTRACE_RESPONSE, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_SCOPES_RESPONSE, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_VARIABLES_RESPONSE, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_BREAKPOINT_LOCATIONS_RESPONSE, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_CONFIGURARIONE_DONE_RESPONSE, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_SET_SOURCE_BREAKPOINT_RESPONSE, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_SET_EXCEPTION_BREAKPOINT_RESPONSE, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_SET_FUNCTION_BREAKPOINT_RESPONSE, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_LAUNCH_RESPONSE, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_THREADS_RESPONSE, DAPEvent);

wxDEFINE_EVENT(wxEVT_DAP_RUN_IN_TERMINAL_REQUEST, DAPEvent);

wxDEFINE_EVENT(wxEVT_DAP_STOPPED_EVENT, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_STOPPED_ON_ENTRY_EVENT, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_PROCESS_EVENT, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_EXITED_EVENT, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_TERMINATED_EVENT, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_INITIALIZED_EVENT, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_OUTPUT_EVENT, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_BREAKPOINT_EVENT, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_CONTINUED_EVENT, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_MODULE_EVENT, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_DEBUGPYWAITINGFORSERVER_EVENT, DAPEvent);

DAPEvent::DAPEvent(wxEventType commandType, int winid)
    : wxCommandEvent(commandType, winid)
{
}
DAPEvent::~DAPEvent() {}
DAPEvent::DAPEvent(const DAPEvent& event) { *this = event; }
DAPEvent& DAPEvent::operator=(const DAPEvent& src)
{
    m_object = src.m_object;
    return *this;
}

wxEvent* DAPEvent::Clone() const { return new DAPEvent(*this); }
dap::Event* DAPEvent::GetDapEvent() const
{
    if(!m_object) {
        return nullptr;
    }
    return m_object->As<dap::Event>();
}

dap::Response* DAPEvent::GetDapResponse() const
{
    if(!m_object) {
        return nullptr;
    }
    return m_object->As<dap::Response>();
}

dap::Request* DAPEvent::GetDapRequest() const
{
    if(!m_object) {
        return nullptr;
    }
    return m_object->As<dap::Request>();
}
