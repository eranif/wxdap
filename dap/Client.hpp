#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "DAPEvent.hpp"
#include "JsonRPC.hpp"
#include "Socket.hpp"
#include "dap_exports.hpp"

#include <atomic>
#include <functional>
#include <queue>
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

typedef std::function<void(bool, const wxString&, const wxString&)> source_loaded_cb;
typedef std::function<void(bool, const wxString&, const wxString&, int)> evaluate_cb;

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
    bool m_can_interact = false;
    std::unordered_map<size_t, wxString> m_requestIdToFilepath;
    size_t m_features = 0;

    /// set this to true if you wish to receive (in addition to the regular events)
    /// logging events that can be used to trace the protocol exchange between
    /// wxdap and the dap-server
    bool m_wants_log_events = false;

    /// the ID if thread that called GetFrames()
    std::vector<int> m_get_frames_queue;
    std::vector<int> m_get_scopes_queue;
    std::vector<std::pair<int, EvaluateContext>> m_get_variables_queue;
    std::vector<source_loaded_cb> m_load_sources_queue;
    std::vector<evaluate_cb> m_evaluate_queue;
    std::vector<wxString> m_source_breakpoints_queue;
    std::unordered_map<int, dap::Request*> m_in_flight_requests;

protected:
    bool IsSupported(eFeatures feature) const { return m_features & feature; }
    bool SendRequest(dap::Request* request);
    void HandleSourceResponse(Json json);
    void HandleEvaluateResponse(Json json);
    /// Return the originating request for `response`
    /// Might return null
    dap::Request* GetOriginatingRequest(dap::Response* response);

protected:
    void SendDAPEvent(wxEventType type, ProtocolMessage* dap_message, Json json, Request* req);

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
     * @brief handle Json payload received from the DAP server
     * @param json
     */
    void OnMessage(Json json);
    static void StaticOnDataRead(Json json, wxObject* o);

public:
    Client();
    virtual ~Client();

    /**
     * @brief enable/disable logging events
     */
    void SetWantsLogEvents(bool b) { m_wants_log_events = b; }

    template <typename RequestType>
    RequestType* MakeRequest()
    {
        RequestType* req = new RequestType();
        req->seq = GetNextSequence();
        return req;
    }

    /**
     * @brief send back a response to the dap server
     * should be used when receiving a reverse request from the dap server
     */
    bool SendResponse(dap::Response& response);

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
    void Initialize(const dap::InitializeRequestArguments* initArgs = nullptr);

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
     * @param workingDirectory the debuggee working directory
     * @param env extra environment variable to pass to the debuggee
     */
    void Launch(std::vector<wxString>&& cmd, const wxString& workingDirectory = wxEmptyString,
                const dap::Environment& env = {});
    /**
     * @brief attach to dap server
     */
    void Attach(int pid = wxNOT_FOUND, const std::vector<wxString>& arguments = {});

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
    void Continue(int threadId = wxNOT_FOUND, bool all_threads = true);

    /**
     * @brief The request executes one step (in the given granularity) for the specified thread and allows all other
     * threads to run freely by resuming them
     * @param threadId execute one step for this thread. If wxNOT_FOUND is passed, use the thread returned by
     * GetActiveThreadId()
     * @param singleThread If this optional flag is true, execution is resumed only for the thread
     */
    void Next(int threadId = wxNOT_FOUND, bool singleThread = true,
              SteppingGranularity granularity = SteppingGranularity::LINE);

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
    void StepIn(int threadId = wxNOT_FOUND, bool singleThread = true);

    /**
     * @brief step out of a function
     */
    void StepOut(int threadId = wxNOT_FOUND, bool singleThread = true);

    /**
     * @brief return the list of all children variables for `variablesReference`
     * @param variablesReference the parent ID
     * @param context the context of variablesReference
     * @param count number of children. If count 0, all variables are returned
     */
    void GetChildrenVariables(int variablesReference, EvaluateContext context = EvaluateContext::VARIABLES,
                              size_t count = 10, ValueDisplayFormat format = ValueDisplayFormat::NATIVE);

    /**
     * @brief The request suspends the debuggee.
     * @param threadId
     */
    void Pause(int threadId = wxNOT_FOUND);

    /**
     * @brief request list of all breakpoints in a file
     */
    void BreakpointLocations(const wxString& filepath, int start_line, int end_line);

    /**
     * @brief request to load a source and execute callback once the source is loaded. If the `source` contains
     * a local path, the callback is called immediately without accessing the server, if the `source` contains
     * sourceReference, then this function queues a load source request to the server and executes the callback once
     * the source is loaded
     */
    bool LoadSource(const dap::Source& source, source_loaded_cb callback);

    /**
     * @brief evaluate an expression. This method uses callback instead of event since a context is required
     * (e.g. the caller want to associate the evaluated expression with the expression)
     *
     * evaluate_cb(bool success, const wxString& result, const wxString& type, int variablesReference):
     * - success: the evaluate succeeded
     * - result: The result of the evaluate request
     * - type: The type of the evaluate result
     * - variablesReference: If variablesReference is > 0, the evaluate result is structured and its
     *   children can be retrieved by passing variablesReference to the
     *   VariablesRequest
     */
    void EvaluateExpression(const wxString& expression, int frameId, EvaluateContext context, evaluate_cb callback,
                            ValueDisplayFormat format = ValueDisplayFormat::NATIVE);
};

};     // namespace dap
#endif // CLIENT_HPP
