#include "DAPEvent.hpp"

wxDEFINE_EVENT(wxEVT_DAP_STOPPED_EVENT, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_PROCESS_EVENT, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_INITIALIZE_RESPONSE, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_STACKTRACE_RESPONSE, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_EXITED_EVENT, DAPEvent);
wxDEFINE_EVENT(wxEVT_DAP_TERMINATED_EVENT, DAPEvent);

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
