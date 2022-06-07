#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "JsonRPC.hpp"
#include "Socket.hpp"

#include "DAPEvent.hpp"
#include "dap_exports.hpp"
#include <wx/event.h>
#include <wx/string.h>

namespace dap
{
class WXDLLIMPEXP_DAP Client : public wxEvtHandler
{
    enum class eHandshakeState { kNotPerformed, kInProgress, kCompleted };

    Socket::Ptr_t m_socket = nullptr;
    dap::JsonRPC m_rpc;
    atomic_bool m_shutdown;
    atomic_bool m_terminated;
    thread* m_readerThread = nullptr;
    size_t m_requestSeuqnce = 0;
    eHandshakeState m_handshake_state = eHandshakeState::kNotPerformed;
    int m_active_thread_id = wxNOT_FOUND;

protected:
    size_t GetNextSequence()
    {
        m_requestSeuqnce++;
        return m_requestSeuqnce;
    }

    void SendDAPEvent(wxEventType type, ProtocolMessage* dap_message, JSON json);

    /**
     * @brief we maintain a reader thread that is responsible for reading
     * from the socket and post events when new messages arrived
     */
    void StartReaderThread();

    /**
     * @brief stop the reader thread if needed and clean any resources allocted
     */
    void StopReaderThread();

    /**
     * @brief this callback is called by the reader thread whenever data arrives on the socket
     */
    void OnDataRead(const wxString& buffer);

    /**
     * @brief handle JSON payload received from the DAP server
     * @param json
     */
    void OnJsonRead(JSON json);
    static void StaticOnJsonRead(JSON json, wxObject* o);

public:
    Client();
    virtual ~Client();

    /**
     * @brief return the currently active thread ID
     */
    int GetActiveThreadId() const { return m_active_thread_id; }

    /**
     * @brief Wait until connection is established
     */
    bool Connect(const wxString& connection_string, int timeoutSeconds);

    /**
     * @brief initiate the handshake between the server and the client
     */
    void Initialize();

    /**
     * @brief are we still connected?
     */
    bool IsConnected() const;

    /**
     * @brief set multiple breakpoints in a source file
     */
    void SetBreakpointsFile(const wxString& file, const std::vector<dap::SourceBreakpoint>& lines);

    /**
     * @brief tell the debugger that we are done and ready to start the main loop
     */
    void ConfigurationDone();
    /**
     * @brief start the debuggee
     */
    void Launch(std::vector<wxString> cmd);

    /**
     * @brief ask for list of threads
     */
    void GetThreads();

    /**
     * @brief get list of frames for a given thread ID
     * @param threadId
     * @param starting_frame
     * @param frame_count number of frames to return
     */
    void GetFrames(int threadId, int starting_frame = 0, int frame_count = 0);

    /**
     * @brief continue execution
     */
    void Continue();

    /**
     * @brief The request executes one step (in the given granularity) for the specified thread and allows all other
     * threads to run freely by resuming them
     */
    void Next(int threadId);

    /**
     * @brief return the variables for stack frame
     * @param frameId
     */
    void GetScopes(int frameId);
};

};     // namespace dap
#endif // CLIENT_HPP
