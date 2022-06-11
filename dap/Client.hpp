#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "DAPEvent.hpp"
#include "JsonRPC.hpp"
#include "Socket.hpp"
#include "dap_exports.hpp"

#include <atomic>
#include <wx/event.h>
#include <wx/string.h>

namespace dap
{
/// The transport class used to communicate with the DAP server
/// Note about thread safety:
/// This class must be stateless and thread-safe since it is
/// used by both an internal thread and the caller thread
class WXDLLIMPEXP_DAP Transport
{
public:
    Transport() {}
    virtual ~Transport() {}

    /**
     * @brief return from the network with a given timeout
     * @returns true on success, false in case of an error. True is also returned when timeout occurs, check the buffer
     * length if it is 0, timeout occured
     */
    virtual bool Read(wxString& WXUNUSED(buffer), int msTimeout) = 0;

    /**
     * @brief send data over the network
     * @return number of bytes written
     */
    virtual size_t Send(const wxString& WXUNUSED(buffer)) = 0;
};

/// simple socket implementation for Socket
class WXDLLIMPEXP_DAP SocketTransport : public Transport
{
    Socket* m_socket = nullptr;

public:
    SocketTransport();
    virtual ~SocketTransport();

    bool Read(wxString& buffer, int msTimeout) override;
    size_t Send(const wxString& buffer) override;

    // socket specific
    bool Connect(const wxString& connection_string, int timeoutSeconds);
};

class WXDLLIMPEXP_DAP Client : public wxEvtHandler
{
    enum eFeatures {
        supportsConfigurationDoneRequest = (1 << 0),
        supportsFunctionBreakpoints = (1 << 1),
        supportsConditionalBreakpoints = (1 << 2),
        supportsHitConditionalBreakpoints = (1 << 3),
        supportsEvaluateForHovers = (1 << 4),
        supportsStepBack = (1 << 5),
        supportsSetVariable = (1 << 6),
        supportsRestartFrame = (1 << 7),
        supportsGotoTargetsRequest = (1 << 8),
        supportsStepInTargetsRequest = (1 << 9),
        supportsCompletionsRequest = (1 << 10),
        supportsModulesRequest = (1 << 11),
        supportsRestartRequest = (1 << 12),
        supportsExceptionOptions = (1 << 13),
        supportsValueFormattingOptions = (1 << 14),
        supportsExceptionInfoRequest = (1 << 15),
        supportTerminateDebuggee = (1 << 16),
        supportsDelayedStackTraceLoading = (1 << 17),
        supportsLoadedSourcesRequest = (1 << 18),
        supportsProgressReporting = (1 << 19),
        supportsRunInTerminalRequest = (1 << 20),
        supportsBreakpointLocationsRequest = (1 << 21),
    };

protected:
    enum class eHandshakeState { kNotPerformed, kInProgress, kCompleted };
    Transport* m_transport = nullptr;
    dap::JsonRPC m_rpc;
    std::atomic_bool m_shutdown;
    std::atomic_bool m_terminated;
    std::thread* m_readerThread = nullptr;
    size_t m_requestSeuqnce = 0;
    eHandshakeState m_handshake_state = eHandshakeState::kNotPerformed;
    int m_active_thread_id = wxNOT_FOUND;
    bool m_waiting_for_stopped_on_entry = false;
    bool m_can_interact = false;
    std::unordered_map<size_t, wxString> m_requestIdToFilepath;
    size_t m_features = 0;
    bool m_stopOnEntry = true;

protected:
    bool IsSupported(eFeatures feature) const { return m_features & feature; }
    bool SendRequest(dap::ProtocolMessage& request);

protected:
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
     * @brief lost connection to the DAP server
     */
    void OnConnectionError();

    /**
     * @brief handle JSON payload received from the DAP server
     * @param json
     */
    void OnMessage(JSON json);
    static void StaticOnDataRead(JSON json, wxObject* o);

public:
    Client();
    virtual ~Client();

    void SetStopOnEntry(bool stopOnEntry) { this->m_stopOnEntry = stopOnEntry; }
    bool IsStopOnEntry() const { return m_stopOnEntry; }

    /**
     * @brief return the next message sequence
     */
    size_t GetNextSequence()
    {
        m_requestSeuqnce++;
        return m_requestSeuqnce;
    }

    /**
     * @brief set the tranposrt for this client. The `Client` takes
     * the ownership for this pointer and will free it when its done with it.
     * This means that transport **must** be allocated on the heap
     */
    void SetTransport(dap::Transport* transport);

    /**
     * @brief can we interact with the debugger?
     */
    bool CanInteract() const { return m_can_interact; }

    /**
     * @brief return the currently active thread ID
     */
    int GetActiveThreadId() const { return m_active_thread_id; }

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
    void SetFunctionBreakpoints(const std::vector<dap::FunctionBreakpoint>& breakpoints);

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
    void Launch(std::vector<wxString>&& cmd, const wxString& workingDirectory = wxEmptyString, bool stopOnEntry = false,
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
     * @brief return the variable scopes for a given frame
     * @param frameId
     */
    void GetScopes(int frameId);

    /**
     * @brief reset the session and clear all states
     */
    void Reset();

    /**
     * @brief step into function
     */
    void StepIn(int threadId = wxNOT_FOUND);

    /**
     * @brief step out of a function
     */
    void StepOut(int threadId = wxNOT_FOUND);

    /**
     * @brief return the list of all children variables for `variablesReference`
     * @param variablesReference the parent ID
     * @param count number of children. If count 0, all variables are returned
     */
    void GetChildrenVariables(int variablesReference, size_t count = 10, const wxString& format = wxEmptyString);

    /**
     * @brief The request suspends the debuggee.
     * @param threadId
     */
    void Pause(int threadId = wxNOT_FOUND);

    /**
     * @brief request list of all breakpoints in a file
     */
    void BreakpointLocations(const wxString& filepath, int start_line, int end_line);
};

};     // namespace dap
#endif // CLIENT_HPP
