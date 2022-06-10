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
    bool m_waiting_for_stopped_on_entry = false;
    bool m_can_interact = false;

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
     * @brief can we interact with the debugger?
     */
    bool CanInteract() const { return m_can_interact; }

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
     * @brief set breakpoint on a function
     */
    void SetFunctionBreakpoints(const wxString& function);

    /**
     * @brief tell the debugger that we are done and ready to start the main loop
     */
    void ConfigurationDone();
    /**
     * @brief start the debuggee
     * @param cmd the cmd in [0] is the program, the remainder are the arguments
     * @param workingDirectory working directory, if empty, use `wxGetCwd()`
     * @param stopOnEntry stop the debugger as soon as the debug session starts
     * @param env array of strings in the form of `{ "A=B", "C=D", ... }`
     */
    void Launch(std::vector<wxString>&& cmd, const wxString& workingDirectory, bool stopOnEntry,
                const std::vector<wxString>& env = {});

    /**
     * @brief ask for list of threads
     */
    void GetThreads();

    /**
     * @brief get list of frames for a given thread ID
     * @param threadId if wxNOT_FOUND is specified, use the thread ID as returned by GetActiveThreadId()
     * @param starting_frame
     * @param frame_count number of frames to return
     */
    void GetFrames(int threadId = wxNOT_FOUND, int starting_frame = 0, int frame_count = 0);

    /**
     * @brief continue execution
     */
    void Continue();

    /**
     * @brief The request executes one step (in the given granularity) for the specified thread and allows all other
     * threads to run freely by resuming them
     * @param threadId execute one step for this thread. If wxNOT_FOUND is passed, use the thread returned by
     * GetActiveThreadId()
     */
    void Next(int threadId = wxNOT_FOUND);

    /**
     * @brief return the variables for stack frame
     * @param frameId
     */
    void GetScopes(int frameId);

    /**
     * @brief reset the session and clear all states
     */
    void Cleanup();

    /**
     * @brief step into function
     */
    void StepIn(int threadId = wxNOT_FOUND);

    /**
     * @brief step out of a function
     */
    void StepOut(int threadId = wxNOT_FOUND);
};

};     // namespace dap
#endif // CLIENT_HPP
