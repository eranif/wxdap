#ifndef PROTOCOLMESSAGE_HPP
#define PROTOCOLMESSAGE_HPP

#include "JSON.hpp"
#include "dap_exports.hpp"

#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <wx/any.h>
#include <wx/string.h>

#if wxVERSION_NUMBER < 3100
namespace std
{
template <>
struct hash<wxString> {
    std::size_t operator()(const wxString& s) const { return hash<std::wstring>{}(s.ToStdWstring()); }
};
} // namespace std
#endif

/// C++ Implementation of Debug Adapter Protocol (DAP)
/// The Debug Adapter Protocol defines the protocol used between an editor or
/// IDE and a debugger or runtime The implementation is based on the
/// specifications described here:
/// https://microsoft.github.io/debug-adapter-protocol/specification
#define JSON_SERIALIZE()      \
    Json To() const override; \
    void From(const Json& json) override

#define REQUEST_CLASS(Type, Command)                              \
    Type()                                                        \
    {                                                             \
        command = Command;                                        \
        ObjGenerator::Get().RegisterRequest(Command, &Type::New); \
    }                                                             \
    virtual ~Type() {}                                            \
    static ProtocolMessage::Ptr_t New() { return ProtocolMessage::Ptr_t(new Type()); }

#define RESPONSE_CLASS(Type, Command)                              \
    Type()                                                         \
    {                                                              \
        command = Command;                                         \
        ObjGenerator::Get().RegisterResponse(Command, &Type::New); \
    }                                                              \
    virtual ~Type() {}                                             \
    static ProtocolMessage::Ptr_t New() { return ProtocolMessage::Ptr_t(new Type()); }

#define EVENT_CLASS(Type, Command)                              \
    Type()                                                      \
    {                                                           \
        event = Command;                                        \
        ObjGenerator::Get().RegisterEvent(Command, &Type::New); \
    }                                                           \
    virtual ~Type() {}                                          \
    static ProtocolMessage::Ptr_t New() { return ProtocolMessage::Ptr_t(new Type()); }

/// Register the class Type by creating a global dummy instance so the constructor will get called
/// the registration is done in the ctor
#define REGISTER_CLASS(Type) Type dummy_##Type

#define ANY_CLASS(Type) \
    Type() {}           \
    virtual ~Type() {}

#define PTR_SIZE (sizeof(void*))
namespace dap
{
void WXDLLIMPEXP_DAP Initialize();

/// Enviroment format
/// some adapters (e.g. codelldb) accpets the environment in the form of:
/// {"ENV_1": "value", "ENV_2": 1}
/// and others (e.g. lldb-vscode) uses this format:
/// ["ENV_1=value", "ENV_2=1]
enum class EnvFormat {
    DICTIONARY,
    LIST,
    NONE, // the adapter does not accept env
};

enum class SteppingGranularity {
    LINE,
    STATEMENT,
    INSTRUCTION,
};

enum class ValueDisplayFormat {
    NATIVE,
    HEX,
};

enum class EvaluateContext {
    VARIABLES,
    WATCH,
    REPL,
    HOVER,
    CLIPBOARD,
};

struct WXDLLIMPEXP_DAP Environment {
    EnvFormat format = EnvFormat::DICTIONARY;
    std::unordered_map<wxString, wxString> vars;

    Json To() const;
    void From(const Json& json);
};

// base class representing anything

struct WXDLLIMPEXP_DAP Any {
    Any() {}
    virtual ~Any() {}

    virtual Json To() const = 0;
    virtual void From(const Json& json) = 0;

    template <typename T>
    T* As() const
    {
        return dynamic_cast<T*>(const_cast<Any*>(this));
    }
};

struct Event;
struct Request;
struct Response;

/// Base class of requests, responses, and events
struct WXDLLIMPEXP_DAP ProtocolMessage : public Any {
    int seq = -1;
    wxString type;
    typedef std::shared_ptr<ProtocolMessage> Ptr_t;

    dap::Event* AsEvent() const
    {
        if(type != "event") {
            return nullptr;
        } else {
            return As<dap::Event>();
        }
    }
    dap::Request* AsRequest() const
    {
        if(type != "request") {
            return nullptr;
        } else {
            return As<dap::Request>();
        }
    }

    dap::Response* AsResponse() const
    {
        if(type != "response") {
            return nullptr;
        } else {
            return As<dap::Response>();
        }
    }

    wxString ToString() const;
    ANY_CLASS(ProtocolMessage);
    JSON_SERIALIZE();
};

class WXDLLIMPEXP_DAP ObjGenerator
{
    typedef std::function<ProtocolMessage::Ptr_t()> onNewObject;
    std::unordered_map<wxString, onNewObject> m_responses;
    std::unordered_map<wxString, onNewObject> m_events;
    std::unordered_map<wxString, onNewObject> m_requests;

protected:
    ProtocolMessage::Ptr_t New(const wxString& name, const std::unordered_map<wxString, onNewObject>& pool);

public:
    static ObjGenerator& Get();
    /**
     * @brief create new ProtocolMessage.
     * @param type can be one of ["response", "event", "request"] anything else will return nullptr
     * @param name the class name
     * @return
     */
    ProtocolMessage::Ptr_t New(const wxString& type, const wxString& name);

    /**
     * @brief create new ProtocolMessage from raw Json object
     */
    ProtocolMessage::Ptr_t FromJSON(Json json);

    void RegisterResponse(const wxString& name, onNewObject func);
    void RegisterEvent(const wxString& name, onNewObject func);
    void RegisterRequest(const wxString& name, onNewObject func);
};

/// A client or debug adapter initiated request
/// ->
struct WXDLLIMPEXP_DAP Request : public ProtocolMessage {
    wxString command;

    Request() { type = "request"; }
    virtual ~Request() = 0; // force to abstract class
    JSON_SERIALIZE();
};

/// The 'cancel' request is used by the frontend to indicate that it is no
/// longer interested in the result produced by a specific request issued
/// earlier.
/// <->
struct WXDLLIMPEXP_DAP CancelRequest : public Request {
    int requestId = -1;
    REQUEST_CLASS(CancelRequest, "cancel");
    JSON_SERIALIZE();
};

/// A debug adapter initiated event
///
struct WXDLLIMPEXP_DAP Event : public ProtocolMessage {
    wxString event;
    Event() { type = "event"; }
    virtual ~Event() = 0; // force to abstract class
    JSON_SERIALIZE();
};

/// Response for a request
/// <-
struct WXDLLIMPEXP_DAP Response : public ProtocolMessage {
    int request_seq = -1;
    bool success = true;
    wxString command;

    /**
     * Contains the raw error in short form if 'success' is false.
     * This raw error might be interpreted by the frontend and is not shown in
     * the UI. Some predefined values exist. Values: 'cancelled': request was
     * cancelled. etc.
     */
    wxString message;

    Response() { type = "response"; }
    virtual ~Response() = 0; // force to abstract class
    JSON_SERIALIZE();
};

/// Response to 'cancel' request. This is just an acknowledgement, so no body
/// field is required.
/// <-
struct WXDLLIMPEXP_DAP CancelResponse : public Response {
    RESPONSE_CLASS(CancelResponse, "cancel");
    Json To() const override { return Response::To(); }
    void From(const Json& json) override { Response::From(json); }
};

/// This event indicates that the debug adapter is ready to accept configuration
/// requests (e.g. SetBreakpointsRequest, SetExceptionBreakpointsRequest).
/// <-
struct WXDLLIMPEXP_DAP InitializedEvent : public Event {
    EVENT_CLASS(InitializedEvent, "initialized");
    JSON_SERIALIZE();
};

/// The event indicates that the execution of the debuggee has stopped due to
/// some condition. This can be caused by a break point previously set, a
/// stepping action has completed, by executing a debugger statement etc.
struct WXDLLIMPEXP_DAP StoppedEvent : public Event {
    /**
     * The reason for the event.
     * For backward compatibility this wxString is shown in the UI if the
     * 'description' attribute is missing (but it must not be translated).
     * Values: 'step', 'breakpoint', 'exception', 'pause', 'entry', 'goto',
     * 'function breakpoint', 'data breakpoint', etc.
     */
    wxString reason;
    /**
     * Additional information. E.g. if reason is 'exception', text contains the
     * exception name. This wxString is shown in the UI.
     */
    wxString text;
    /**
     * The full reason for the event, e.g. 'Paused on exception'. This string is
     * shown in the UI as is and must be translated.
     */
    wxString description;
    /**
     * If 'allThreadsStopped' is true, a debug adapter can announce that all
     * threads have stopped.
     * - The client should use this information to enable that all threads can
     * be expanded to access their stacktraces.
     * - If the attribute is missing or false, only the thread with the given
     * threadId can be expanded.
     */
    bool allThreadsStopped = false;
    /**
     * The thread which was stopped.
     */
    int threadId = wxNOT_FOUND;
    EVENT_CLASS(StoppedEvent, "stopped");
    JSON_SERIALIZE();
};

/// The event indicates that the execution of the debuggee has continued.
/// Please note: a debug adapter is not expected to send this event in response
/// to a request that implies that execution continues, e.g. 'launch' or
/// 'continue'.  It is only necessary to send a 'continued' event if there was
/// no previous request that implied this
struct WXDLLIMPEXP_DAP ContinuedEvent : public Event {
    /**
     * The thread which was continued.
     */
    int threadId = -1;
    /**
     * If 'allThreadsContinued' is true, a debug adapter can announce that all
     * threads have continued.
     */
    bool allThreadsContinued = true;
    EVENT_CLASS(ContinuedEvent, "continued");
    JSON_SERIALIZE();
};

/// The event indicates that the debuggee has exited and returns its exit code.
struct WXDLLIMPEXP_DAP ExitedEvent : public Event {
    int exitCode = 0;
    EVENT_CLASS(ExitedEvent, "exited");
    JSON_SERIALIZE();
};

/// The event indicates that debugging of the debuggee has terminated. This does
/// not mean that the debuggee itself has exited
struct WXDLLIMPEXP_DAP TerminatedEvent : public Event {
    EVENT_CLASS(TerminatedEvent, "terminated");
    JSON_SERIALIZE();
};

/// The event indicates that a thread has started or exited.
struct WXDLLIMPEXP_DAP ThreadEvent : public Event {
    wxString reason;
    int threadId = 0;
    EVENT_CLASS(ThreadEvent, "thread");
    JSON_SERIALIZE();
};

/// The event indicates that a thread has started or exited.
// <-
struct WXDLLIMPEXP_DAP OutputEvent : public Event {
    /**
     * The output category. If not specified, 'console' is assumed.
     * Values: 'console', 'stdout', 'stderr', 'telemetry', etc.
     */
    wxString category;
    wxString output;

    EVENT_CLASS(OutputEvent, "output");
    JSON_SERIALIZE();
};

/// A Source is a descriptor for source code. It is returned from the debug
/// adapter as part of a StackFrame and it is used by clients when specifying
/// breakpoints
struct WXDLLIMPEXP_DAP Source : public Any {
    /**
     * The short name of the source. Every source returned from the debug
     * adapter has a name. When sending a source to the debug adapter this name
     * is optional.
     */
    wxString name;
    /**
     * The path of the source to be shown in the UI. It is only used to locate
     * and load the content of the source if no sourceReference is specified (or
     * its value is 0).
     */
    wxString path;
    /**
     * If sourceReference > 0 the contents of the source must be retrieved through
     * the SourceRequest (even if a path is specified).
     * A sourceReference is only valid for a session, so it must not be used to
     * persist a source.
     * The value should be less than or equal to 2147483647 (2^31-1).
     */
    int sourceReference = 0;

    bool operator==(const Source& other) const
    {
        return name == other.name && path == other.path && sourceReference == other.sourceReference;
    }

    ANY_CLASS(Source);
    JSON_SERIALIZE();
};

/// Information about a Breakpoint created in setBreakpoints or
/// setFunctionBreakpoints.
struct WXDLLIMPEXP_DAP Breakpoint : public Any {
    int id = -1;
    bool verified = false;
    wxString message;
    Source source;
    int line = -1;
    int column = -1;
    int endLine = -1;
    int endColumn = -1;

    /// implement simple operator==
    bool operator==(const Breakpoint& other) const;

    ANY_CLASS(Breakpoint);
    JSON_SERIALIZE();
};

/// A Module object represents a row in the modules view
struct WXDLLIMPEXP_DAP Module : public Any {
    /**
     * Unique identifier for the module.
     */
    wxString id;
    /**
     * A name of the module.
     */
    wxString name;
    /**
     * optional but recommended attributes.
     * always try to use these first before introducing additional attributes.
     *
     * Logical full path to the module. The exact definition is implementation
     * defined, but usually this would be a full path to the on-disk file for the
     * module.
     */
    wxString path;
    /**
     * True if the module is optimized.
     */
    bool isOptimized = false;
    /**
     * True if the module is considered 'user code' by a debugger that supports
     * 'Just My Code'.
     */
    bool isUserCode = false;
    /**
     * Version of Module.
     */
    wxString version;
    /**
     * User understandable description of if symbols were found for the module
     * (ex: 'Symbols Loaded', 'Symbols not found', etc.
     */
    wxString symbolStatus;
    /**
     * Logical full path to the symbol file. The exact definition is
     * implementation defined.
     */
    wxString symbolFilePath;
    /**
     * Module created or modified.
     */
    wxString dateTimeStamp;
    /**
     * Address range covered by this module.
     */
    wxString addressRange;
    ANY_CLASS(Module);
    JSON_SERIALIZE();
};

/// The event indicates that some information about a module has changed
struct WXDLLIMPEXP_DAP ModuleEvent : public Event {
    wxString reason;
    Module module;
    EVENT_CLASS(ModuleEvent, "module");
    JSON_SERIALIZE();
};

/// The event indicates that some information about a breakpoint has changed.
// <-
struct WXDLLIMPEXP_DAP BreakpointEvent : public Event {
    /**
     * The output category. If not specified, 'console' is assumed.
     * Values: 'console', 'stdout', 'stderr', 'telemetry', etc.
     */
    wxString reason;
    /**
     * The 'id' attribute is used to find the target breakpoint and the other
     * attributes are used as the new values.
     */
    Breakpoint breakpoint;
    EVENT_CLASS(BreakpointEvent, "breakpoint");
    JSON_SERIALIZE();
};

/// The event indicates that the debugger has begun debugging a new process.
/// Either one that it has launched, or one that it has attached to
struct WXDLLIMPEXP_DAP ProcessEvent : public Event {
    /**
     * The logical name of the process. This is usually the full path to
     * process's executable file. Example: /home/example/myproj/program.exe
     */
    wxString name;
    /**
     * The system process id of the debugged process. This property will be
     * missing for non-system processes.
     */
    int systemProcessId = -1;
    /**
     * If true, the process is running on the same computer as the debug
     * adapter.
     */
    bool isLocalProcess = true;
    /**
     * Describes how the debug engine started debugging this process.
     * 'launch': Process was launched under the debugger.
     * 'attach': Debugger attached to an existing process.
     * 'attachForSuspendedLaunch': A project launcher component has launched a
     * new process in a suspended state and then asked the debugger to attach.
     */
    wxString startMethod;
    int pointerSize = PTR_SIZE;
    EVENT_CLASS(ProcessEvent, "process");
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP InitializeRequestArguments : public Any {
    /**
     * The ID of the (frontend) client using this adapter.
     */
    wxString clientID;
    /**
     * The human readable name of the (frontend) client using this adapter.
     */
    wxString clientName;
    /**
     * The ID of the debug adapter.
     */
    wxString adapterID;
    /**
     * The ISO-639 locale of the (frontend) client using this adapter, e.g.
     * en-US or de-CH.
     */
    wxString locale = "en-US";
    bool linesStartAt1 = false;
    bool columnsStartAt1 = false;
    /**
     * Client supports the invalidated event.
     */
    bool supportsInvalidatedEvent = false;
    /**
     * Determines in what format paths are specified. The default is 'path',
     * which is the native format. Values: 'path', 'uri', etc.
     */
    wxString pathFormat = "path";
    ANY_CLASS(InitializeRequestArguments);
    JSON_SERIALIZE();
};

/// The 'initialize' request is sent as the first request from the client to the
/// debug adapter in order to configure it with client capabilities and to
/// retrieve capabilities from the debug adapter.  Until the debug adapter has
/// responded to with an 'initialize' response, the client must not send any
/// additional requests or events to the debug adapter. In addition the debug
/// adapter is not allowed to send any requests or events to the client until it
/// has responded with an 'initialize' response.  The 'initialize' request may
/// only be sent once.
/// <->
struct WXDLLIMPEXP_DAP InitializeRequest : public Request {
    InitializeRequestArguments arguments;
    REQUEST_CLASS(InitializeRequest, "initialize");
    JSON_SERIALIZE();
};

/// A ColumnDescriptor specifies what module attribute to show in a column of the ModulesView,
/// how to format it, and what the column’s label should be.
/// It is only used if the underlying UI actually supports this level of customization.
struct WXDLLIMPEXP_DAP ColumnDescriptor : public Any  {
    // Name of the attribute rendered in this column.
    wxString attributeName;

    // Header UI label of column.
    wxString label;

    // Format to use for the rendered values in this column. TBD how the format
    // strings looks like.
    wxAny wxformat;     // wxString

    // Datatype of values in this column.  Defaults to 'string' if not specified.
    //
    // Must be one of the following enumeration values:
    // 'string', 'number', 'boolean', 'unixTimestampUTC'
    wxAny type;         // wxString

    // Width of this column in characters (hint only).
    wxAny number;       // int

    ANY_CLASS(ColumnDescriptor);
    JSON_SERIALIZE();
};

/// An ExceptionBreakpointsFilter is shown in the UI as an filter option for
/// configuring how exceptions are dealt with.
struct WXDLLIMPEXP_DAP ExceptionBreakpointsFilter : public Any  {
    // The internal ID of the filter option. This value is passed to the
    // 'setExceptionBreakpoints' request.
    wxString filter;

    // The name of the filter option. This will be shown in the UI.
    wxString label;

    // An optional help text providing additional information about the exception
    // filter. This string is typically shown as a hover and must be translated.
    wxAny description;      // wxString

    // Initial value of the filter option. If not specified a value 'false' is assumed.
    wxAny default_value;        // bool

    // Controls whether a condition can be specified for this filter option. If
    // false or missing, a condition can not be set.
    wxAny supportsCondition;        // bool

    // An optional help text providing information about the condition. This
    // string is shown as the placeholder text for a text box and must be
    // translated.
    wxAny conditionDescription;     // wxString

    ANY_CLASS(ExceptionBreakpointsFilter);
    JSON_SERIALIZE();
};

// Names of checksum algorithms that may be supported by a debug adapter.
//
// Must be one of the following enumeration values:
// 'MD5', 'SHA1', 'SHA256', 'timestamp'
using ChecksumAlgorithm = wxString;

/// Information about the capabilities of a debug adapter.
struct WXDLLIMPEXP_DAP Capabilities {
    // The debug adapter supports the 'configurationDone' request.
    wxAny supportsConfigurationDoneRequest;     // bool

    // The debug adapter supports function breakpoints.
    wxAny supportsFunctionBreakpoints;          // bool

    // The debug adapter supports conditional breakpoints.
    wxAny supportsConditionalBreakpoints;       // bool

    // The debug adapter supports breakpoints that break execution after a
    // specified number of hits.
    wxAny supportsHitConditionalBreakpoints;    // bool

    // The debug adapter supports a (side effect free) evaluate request for data
    // hovers.
    wxAny supportsEvaluateForHovers;            // bool

    // Available exception filter options for the 'setExceptionBreakpoints'
    // request.
    wxAny exceptionBreakpointFilters;           // std::vector<ExceptionBreakpointsFilter>

    // The debug adapter supports stepping back via the 'stepBack' and
    // 'reverseContinue' requests.
    wxAny supportsStepBack;                     // bool

    // The debug adapter supports setting a variable to a value.
    wxAny supportsSetVariable;                  // bool

    // The debug adapter supports restarting a frame.
    wxAny supportsRestartFrame;                 // bool

    // The debug adapter supports the 'gotoTargets' request.
    wxAny supportsGotoTargetsRequest;           // bool

    // The debug adapter supports the 'stepInTargets' request.
    wxAny supportsStepInTargetsRequest;         // bool

    // The debug adapter supports the 'completions' request.
    wxAny supportsCompletionsRequest;           // bool

    // The set of characters that should trigger completion in a REPL. If not
    // specified, the UI should assume the '.' character.
    wxAny completionTriggerCharacters;          // std::vector<wxString>

    // The debug adapter supports the 'modules' request.
    wxAny supportsModulesRequest;               // bool

    // The set of additional module information exposed by the debug adapter.
    wxAny additionalModuleColumns;              // std::vector<ColumnDescriptor>

    // Checksum algorithms supported by the debug adapter.
    wxAny  supportedChecksumAlgorithms;         // std::vector<ChecksumAlgorithm>

    // The debug adapter supports the 'restart' request. In this case a client
    // should not implement 'restart' by terminating and relaunching the adapter
    // but by calling the RestartRequest.
    wxAny supportsRestartRequest;               // bool

    // The debug adapter supports 'exceptionOptions' on the
    // setExceptionBreakpoints request.
    wxAny supportsExceptionOptions;             // bool

    // The debug adapter supports a 'format' attribute on the stackTraceRequest,
    // variablesRequest, and evaluateRequest.
    wxAny supportsValueFormattingOptions;       // bool

    // The debug adapter supports the 'exceptionInfo' request.
    wxAny supportsExceptionInfoRequest;         // bool

    // The debug adapter supports the 'terminateDebuggee' attribute on the
    // 'disconnect' request.
    wxAny supportTerminateDebuggee;             // bool

    // The debug adapter supports the `suspendDebuggee` attribute on the
    // `disconnect` request.
    wxAny supportSuspendDebuggee;               // bool

    // The debug adapter supports the delayed loading of parts of the stack, which
    // requires that both the 'startFrame' and 'levels' arguments and an optional
    // 'totalFrames' result of the 'StackTrace' request are supported.
    wxAny supportsDelayedStackTraceLoading;     // bool

    // The debug adapter supports the 'loadedSources' request.
    wxAny supportsLoadedSourcesRequest;         // bool

    // The debug adapter supports logpoints by interpreting the 'logMessage'
    // attribute of the SourceBreakpoint.
    wxAny supportsLogPoints;                    // bool

    // The debug adapter supports the 'terminateThreads' request.
    wxAny supportsTerminateThreadsRequest;      // bool

    // The debug adapter supports the 'setExpression' request.
    wxAny supportsSetExpression;                // bool

    // The debug adapter supports the 'terminate' request.
    wxAny supportsTerminateRequest;             // bool

    // The debug adapter supports data breakpoints.
    wxAny supportsDataBreakpoints;              // bool

    // The debug adapter supports the 'readMemory' request.
    wxAny supportsReadMemoryRequest;            // bool

    // The debug adapter supports the `writeMemory` request.
    wxAny supportsWriteMemoryRequest;           // bool

    // The debug adapter supports the 'disassemble' request.
    wxAny supportsDisassembleRequest;           // bool

    // The debug adapter supports the 'cancel' request.
    wxAny supportsCancelRequest;                // bool

    // The debug adapter supports the 'breakpointLocations' request.
    wxAny supportsBreakpointLocationsRequest;   // bool

    // The debug adapter supports the 'clipboard' context value in the 'evaluate'
    // request.
    wxAny supportsClipboardContext;             // bool

    // The debug adapter supports stepping granularities (argument 'granularity')
    // for the stepping requests.
    wxAny supportsSteppingGranularity;          // bool

    // The debug adapter supports adding breakpoints based on instruction
    // references.
    wxAny supportsInstructionBreakpoints;       // bool

    // The debug adapter supports 'filterOptions' as an argument on the
    // 'setExceptionBreakpoints' request.
    wxAny supportsExceptionFilterOptions;       // bool

    // The debug adapter supports the `singleThread` property on the execution
    // requests (`continue`, `next`, `stepIn`, `stepOut`, `reverseContinue`,
    // `stepBack`).
    wxAny supportsSingleThreadExecutionRequests;// bool
};

/// Response to 'initialize' request.
/// <-
struct WXDLLIMPEXP_DAP InitializeResponse : public Response {

    // The set of updated capabilities.
    Capabilities capabilities;

    RESPONSE_CLASS(InitializeResponse, "initialize");
    JSON_SERIALIZE();
};

/// The client of the debug protocol must send this request at the end of the
/// sequence of configuration requests (which was started by the 'initialized'
/// event).
/// <->
struct WXDLLIMPEXP_DAP ConfigurationDoneRequest : public Request {
    REQUEST_CLASS(ConfigurationDoneRequest, "configurationDone");
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP EmptyAckResponse : public Response {
    RESPONSE_CLASS(EmptyAckResponse, "");
    JSON_SERIALIZE();
};

/// Response to 'configurationDone' request. This is just an acknowledgement, so
/// no body field is required.
/// <-
struct WXDLLIMPEXP_DAP ConfigurationDoneResponse : public EmptyAckResponse {
    RESPONSE_CLASS(ConfigurationDoneResponse, "configurationDone");
};

/// Arguments for 'launch' request. Additional attributes are implementation
/// specific.
struct WXDLLIMPEXP_DAP LaunchRequestArguments : public Any {
    /**
     * If noDebug is true the launch request should launch the program without
     * enabling debugging.
     */
    bool noDebug = false;

    /// program to launch
    wxString program;

    /// extra arguments to append to the program
    std::vector<wxString> args;

    /// working directory
    wxString cwd;

    /// environment variables
    Environment env;

    ANY_CLASS(LaunchRequestArguments);
    JSON_SERIALIZE();
};

/// The launch request is sent from the client to the debug adapter to start the
/// debuggee with or without debugging (if 'noDebug' is true). Since launching
/// is debugger/runtime specific, the arguments for this request are not part of
/// this specification
struct WXDLLIMPEXP_DAP LaunchRequest : public Request {
    LaunchRequestArguments arguments;
    REQUEST_CLASS(LaunchRequest, "launch");
    JSON_SERIALIZE();
};

/// Response to 'launch' request. This is just an acknowledgement, so no body
/// field is required.
/// <-
struct WXDLLIMPEXP_DAP LaunchResponse : public EmptyAckResponse {
    RESPONSE_CLASS(LaunchResponse, "launch");
};

/// Arguments for 'launch' request. Additional attributes are implementation
/// specific.
struct WXDLLIMPEXP_DAP AttachRequestArguments : public Any {
    int pid = wxNOT_FOUND;
    std::vector<wxString> arguments;
    ANY_CLASS(AttachRequestArguments);
    JSON_SERIALIZE();
};

/// The attach request is sent from the client to the debug adapter to attach to a debuggee that is already running.
struct WXDLLIMPEXP_DAP AttachRequest : public Request {
    AttachRequestArguments arguments;
    REQUEST_CLASS(AttachRequest, "attach");
    JSON_SERIALIZE();
};

/// Response to ‘attach’ request. This is just an acknowledgement, so no body field is required
struct WXDLLIMPEXP_DAP AttachResponse : public EmptyAckResponse {
    RESPONSE_CLASS(AttachResponse, "attach");
};

/// The 'disconnect' request is sent from the client to the debug adapter in
/// order to stop debugging. It asks the debug adapter to disconnect from the
/// debuggee and to terminate the debug adapter. If the debuggee has been
/// started with the 'launch' request, the 'disconnect' request terminates the
/// debuggee. If the 'attach' request was used to connect to the debuggee,
/// 'disconnect' does not terminate the debuggee. This behavior can be
/// controlled with the 'terminateDebuggee' argument (if supported by the debug
/// adapter).
struct WXDLLIMPEXP_DAP DisconnectRequest : public Request {
    bool restart = false;
    bool terminateDebuggee = true;
    REQUEST_CLASS(DisconnectRequest, "disconnect");
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP DisconnectResponse : public EmptyAckResponse {
    RESPONSE_CLASS(DisconnectResponse, "disconnect");
};

struct WXDLLIMPEXP_DAP BreakpointLocationsArguments : public Any {
    Source source;
    int line = -1;
    int column = -1;
    int endLine = -1;
    int endColumn = -1;
    ANY_CLASS(BreakpointLocationsArguments);
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP StepArguments : public Any {
    /**
     * Specifies the thread for which to resume execution for one step-into (of
     * the given granularity).
     */
    int threadId = wxNOT_FOUND;

    /**
     * If this optional flag is true, all other suspended threads are not resumed.
     */
    bool singleThread = true;

    /**
     * Optional granularity to step. If no granularity is specified, a granularity
     * of 'statement' is assumed.
     */
    wxString granularity = "line";

    ANY_CLASS(StepArguments);
    JSON_SERIALIZE();
};

typedef StepArguments StepInArguments;
typedef StepArguments StepOutArguments;

/// Properties of a breakpoint location returned from the 'breakpointLocations'
/// request.
struct WXDLLIMPEXP_DAP BreakpointLocation : public Any {
    int line = -1;
    int column = -1;
    int endLine = -1;
    int endColumn = -1;
    ANY_CLASS(BreakpointLocation);
    JSON_SERIALIZE();
};

/// Response to 'breakpointLocations' request.
/// Contains possible locations for source breakpoints.
struct WXDLLIMPEXP_DAP BreakpointLocationsResponse : public Response {
    wxString filepath; /// wxDAP extension: used to pass the filepath back to the caller
    std::vector<BreakpointLocation> breakpoints;
    RESPONSE_CLASS(BreakpointLocationsResponse, "breakpointLocations");
    JSON_SERIALIZE();
};

/// The 'breakpointLocations' request returns all possible locations for source
/// breakpoints in a given range.
struct WXDLLIMPEXP_DAP BreakpointLocationsRequest : public Request {
    BreakpointLocationsArguments arguments;
    REQUEST_CLASS(BreakpointLocationsRequest, "breakpointLocations");
    JSON_SERIALIZE();
};

/// Properties of a breakpoint or logpoint passed to the setBreakpoints request.
struct WXDLLIMPEXP_DAP SourceBreakpoint : public Any {
    int line = -1;
    /**
     * An optional expression for conditional breakpoints.
     */
    wxString condition;
    SourceBreakpoint() {}
    SourceBreakpoint(int line, const wxString& cond)
    {
        this->line = line;
        this->condition = cond;
    }

    bool operator==(const SourceBreakpoint& other) const
    {
        /// source breakpoint are considered the same if they are on the same line number
        return line == other.line;
    }
    JSON_SERIALIZE();
};

/// Properties of a breakpoint or logpoint passed to the setBreakpoints request.
struct WXDLLIMPEXP_DAP FunctionBreakpoint : public Any {
    /**
     * The name of the function
     */
    wxString name;

    /**
     * An optional expression for conditional breakpoints.
     */
    wxString condition;

    FunctionBreakpoint() {}
    FunctionBreakpoint(const wxString& name, const wxString& cond = wxEmptyString)
    {
        this->name = name;
        this->condition = cond;
    }

    bool operator==(const FunctionBreakpoint& other) const
    {
        /// function breakpoint are considered the same if they have the same method name
        return name == other.name;
    }
    JSON_SERIALIZE();
};

/// Properties of a breakpoint or logpoint passed to the setBreakpoints request.
struct WXDLLIMPEXP_DAP SetBreakpointsArguments : public Any {
    Source source;
    std::vector<SourceBreakpoint> breakpoints;
    JSON_SERIALIZE();
};

/// Properties of a function breakpoint
struct WXDLLIMPEXP_DAP SetFunctionBreakpointsArguments : public Any {
    std::vector<FunctionBreakpoint> breakpoints;
    JSON_SERIALIZE();
};

/// Sets multiple breakpoints for a single source and clears all previous
/// breakpoints in that source. To clear all breakpoint for a source, specify an
/// empty array. When a breakpoint is hit, a 'stopped' event (with reason
/// 'breakpoint') is generated.
struct WXDLLIMPEXP_DAP SetBreakpointsRequest : public Request {
    SetBreakpointsArguments arguments;
    REQUEST_CLASS(SetBreakpointsRequest, "setBreakpoints");
    JSON_SERIALIZE();
};

/// Response to 'setBreakpoints' request.
/// Returned is information about each breakpoint created by this request.
/// This includes the actual code location and whether the breakpoint could be verified.
/// The breakpoints returned are in the same order as the elements of the 'breakpoints'
/// (or the deprecated 'lines') array in the arguments
struct WXDLLIMPEXP_DAP SetBreakpointsResponse : public Response {
    std::vector<Breakpoint> breakpoints;

    /// protocol extension: keep the originating source
    wxString originSource;

    RESPONSE_CLASS(SetBreakpointsResponse, "setBreakpoints");
    JSON_SERIALIZE();
};

// *******************************************************************************************************
// *******************************************************************************************************
// ************************************* START Exception Breakpoints *************************************


/// An ExceptionFilterOptions is used to specify an exception filter together
/// with a condition for the setExceptionsFilter request.
struct ExceptionFilterOptions {
    // ID of an exception filter returned by the 'exceptionBreakpointFilters' capability.
    wxString filterId;

    // An optional expression for conditional exceptions.
    // The exception will break into the debugger if the result of the condition
    // is true.
    wxAny condition;        // wxString
};

/// This enumeration defines all possible conditions when a thrown exception
/// should result in a break. never: never breaks, always: always breaks,
/// unhandled: breaks when exception unhandled,
/// userUnhandled: breaks if the exception is not handled by user code.
///
/// Must be one of the following enumeration values:
/// 'never', 'always', 'unhandled', 'userUnhandled'
using ExceptionBreakMode = wxString;

/// An ExceptionPathSegment represents a segment in a path that is used to match
/// leafs or nodes in a tree of exceptions. If a segment consists of more than
/// one name, it matches the names provided if 'negate' is false or missing or it
/// matches anything except the names provided if 'negate' is true.
struct ExceptionPathSegment {
    // If false or missing this segment matches the names provided, otherwise it
    // matches anything except the names provided.
    wxAny negate;       // bool

    // Depending on the value of 'negate' the names that should match or not
    // match.
    std::vector<wxString> names;
};

/// An ExceptionOptions assigns configuration options to a set of exceptions.
struct ExceptionOptions {
  // Condition when a thrown exception should result in a break.
  ExceptionBreakMode breakMode = "never";

  // A path that selects a single or multiple exceptions in a tree. If 'path' is
  // missing, the whole tree is selected. By convention the first segment of the
  // path is a category that is used to group exceptions in the UI.
  wxAny path;       // std::vector<ExceptionPathSegment>>
};


/// Properties of a SetExceptionBreakpointsArguments passed to the setExceptionBreakpointsRequest  request.
struct WXDLLIMPEXP_DAP SetExceptionBreakpointsArguments  : public Any {
    /**
    * Set of exception filters specified by their ID. The set of all possible
    * exception filters is defined by the `exceptionBreakpointFilters`
    * capability. The `filter` and `filterOptions` sets are additive.
    */
    std::vector<wxString> filters;

    /**
    * Set of exception filters and their options. The set of all possible
    * exception filters is defined by the `exceptionBreakpointFilters`
    * capability. This attribute is only honored by a debug adapter if the
    * corresponding capability `supportsExceptionFilterOptions` is true. The
    * `filter` and `filterOptions` sets are additive.
    *
    *    filterOptions?: ExceptionFilterOptions[];
    */
    wxAny filterOptions;      // std::vector<ExceptionFilterOptions>

    /**
    * Configuration options for selected exceptions.
    * The attribute is only honored by a debug adapter if the corresponding
    * capability `supportsExceptionOptions` is true.
    *
    * exceptionOptions?: ExceptionOptions[];
    */
    wxAny exceptionOptions;         // std::vector<ExceptionOptions>
//    REQUEST_CLASS(SetExceptionBreakpointsArguments, "setExceptionBreakpoints");
    JSON_SERIALIZE();
};


/// The request configures the debugger’s response to thrown exceptions.
/// If an exception is configured to break, a stopped event is fired (with reason exception).
/// Clients should only call this request if the corresponding capability exceptionBreakpointFilters returns one or more filters.
struct WXDLLIMPEXP_DAP SetExceptionBreakpointsRequest : public Request {
    SetExceptionBreakpointsArguments arguments;
    REQUEST_CLASS(SetExceptionBreakpointsRequest, "setExceptionBreakpoints");
    JSON_SERIALIZE();
};


/// Response to setExceptionBreakpoints request.
/// The response contains an array of Breakpoint objects with information about each exception breakpoint or filter.
/// The Breakpoint objects are in the same order as the elements of the filters, filterOptions, exceptionOptions
///  arrays given as arguments. If both filters and filterOptions are given, the returned array must start with
/// filters information first, followed by filterOptions information.
struct WXDLLIMPEXP_DAP SetExceptionBreakpointsResponse  : public Response {
#if 0
//    body?: {
        /**
        * Information about the exception breakpoints or filters.
        * The breakpoints returned are in the same order as the elements of the
        * `filters`, `filterOptions`, `exceptionOptions` arrays in the arguments.
        * If both `filters` and `filterOptions` are given, the returned array must
        * start with `filters` information first, followed by `filterOptions`
        * information.
        */
//        breakpoints?: Breakpoint[];
//    };
#endif
    RESPONSE_CLASS(SetExceptionBreakpointsResponse , "setExceptionBreakpoints");
#if 0
    JSON_SERIALIZE();
#endif
};

// ************************************* FINISH Exception Breakpoints ************************************
// *******************************************************************************************************
// *******************************************************************************************************

struct WXDLLIMPEXP_DAP SetFunctionBreakpointsResponse : public SetBreakpointsResponse {
    RESPONSE_CLASS(SetFunctionBreakpointsResponse, "setFunctionBreakpoints");
};

/// Replaces all existing function breakpoints with new function breakpoints
/// To clear all function breakpoints, specify an empty array
/// When a function breakpoint is hit, a ‘stopped’ event (with reason ‘function breakpoint’) is generated
struct WXDLLIMPEXP_DAP SetFunctionBreakpointsRequest : public Request {
    SetFunctionBreakpointsArguments arguments;
    REQUEST_CLASS(SetFunctionBreakpointsRequest, "setFunctionBreakpoints");
    JSON_SERIALIZE();
};

/// Arguments for the continue request
struct WXDLLIMPEXP_DAP ContinueArguments : public Any {
    /**
     * Continue execution for the specified thread (if possible). If the backend
     * cannot continue on a single thread but will continue on all threads, it
     * should set the 'allThreadsContinued' attribute in the response to true.
     */
    int threadId = -1;
    /**
     * If this optional flag is true, execution is resumed only for the thread
     * with given 'threadId'.
     */
    bool singleThread = false;

    ANY_CLASS(ContinueArguments);
    JSON_SERIALIZE();
};

/// Arguments for the continue request
struct WXDLLIMPEXP_DAP NextArguments : public Any {
    /**
     * Execute 'next' for this thread.
     */
    int threadId = -1;
    wxString granularity = "line";
    bool singleThread = true;

    ANY_CLASS(NextArguments);
    JSON_SERIALIZE();
};

/// The request starts the debuggee to run again.
struct WXDLLIMPEXP_DAP ContinueRequest : public Request {
    ContinueArguments arguments;
    REQUEST_CLASS(ContinueRequest, "continue");
    JSON_SERIALIZE();
};

/// Response to 'continue' request.
struct WXDLLIMPEXP_DAP ContinueResponse : public Response {
    bool allThreadsContinued = true;
    RESPONSE_CLASS(ContinueResponse, "continue");
    JSON_SERIALIZE();
};

/// The request starts the debuggee to run again.
struct WXDLLIMPEXP_DAP NextRequest : public Request {
    NextArguments arguments;
    REQUEST_CLASS(NextRequest, "next");
    JSON_SERIALIZE();
};

/// Response to 'continue' request.
struct WXDLLIMPEXP_DAP NextResponse : public EmptyAckResponse {
    RESPONSE_CLASS(NextResponse, "next");
};

/// Step request (base for In and Out)
struct WXDLLIMPEXP_DAP StepRequest : public Request {
    StepArguments arguments;
    REQUEST_CLASS(StepRequest, "step");
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP StepInRequest : public StepRequest {
    REQUEST_CLASS(StepInRequest, "stepIn");
};

struct WXDLLIMPEXP_DAP StepOutRequest : public StepRequest {
    REQUEST_CLASS(StepOutRequest, "stepOut");
};

/// Step response (base for In and Out)
struct WXDLLIMPEXP_DAP StepResponse : public Response {
    RESPONSE_CLASS(StepResponse, "step");
};

struct WXDLLIMPEXP_DAP StepInResponse : public Response {
    RESPONSE_CLASS(StepInResponse, "stepIn");
};

struct WXDLLIMPEXP_DAP StepOutResponse : public Response {
    RESPONSE_CLASS(StepOutResponse, "stepOut");
};

/// A Stackframe contains the source location
struct WXDLLIMPEXP_DAP StackFrame : public Any {
    /**
     * An identifier for the stack frame. It must be unique across all threads. This id can be used to retrieve the
     * scopes of the frame with the 'scopesRequest' or to restart the execution of a stackframe.
     */
    int id = -1;
    /**
     * The name of the stack frame, typically a method name.
     */
    wxString name;

    /**
     * The optional source of the frame.
     */
    Source source;
    /**
     * The line within the file of the frame. If source is null or doesn't exist, line is 0 and must be ignored.
     */
    int line = 0;
    ANY_CLASS(StackFrame);
    JSON_SERIALIZE();
};

/// The request retrieves a list of all threads.
struct WXDLLIMPEXP_DAP ThreadsRequest : public Request {
    REQUEST_CLASS(ThreadsRequest, "threads");
    JSON_SERIALIZE();
};

/// A Thread
struct WXDLLIMPEXP_DAP Thread : public Any {
    /**
     * Unique identifier for the thread.
     */
    int id = -1;

    /**
     * A name of the thread.
     */
    wxString name;
    ANY_CLASS(Thread);
    JSON_SERIALIZE();
};

/// Response to 'threads' request.
struct WXDLLIMPEXP_DAP ThreadsResponse : public Response {
    std::vector<Thread> threads;
    RESPONSE_CLASS(ThreadsResponse, "threads");
    JSON_SERIALIZE();
};

/// Optional properties of a variable that can be used to determine how to render the variable in the UI.
struct WXDLLIMPEXP_DAP VariablePresentationHint : public Any {
    /**
     * The kind of variable. Before introducing additional values, try to use the listed values.
     * Values:
     * 'property': Indicates that the object is a property.
     * 'method': Indicates that the object is a method.
     * 'class': Indicates that the object is a class.
     * 'data': Indicates that the object is data.
     * 'event': Indicates that the object is an event.
     * 'baseClass': Indicates that the object is a base class.
     * 'innerClass': Indicates that the object is an inner class.
     * 'interface': Indicates that the object is an interface.
     * 'mostDerivedClass': Indicates that the object is the most derived class.
     * 'virtual': Indicates that the object is virtual, that means it is a synthetic object introduced by the adapter
     * for rendering purposes, e.g. an index range for large arrays. 'dataBreakpoint': Indicates that a data breakpoint
     * is registered for the object. etc.
     */
    wxString kind;
    /**
     * Set of attributes represented as an array of strings. Before introducing additional values, try to use the listed
     * values. Values: 'static': Indicates that the object is static. 'constant': Indicates that the object is a
     * constant. 'readOnly': Indicates that the object is read only. 'rawString': Indicates that the object is a raw
     * wxString. 'hasObjectId': Indicates that the object can have an Object ID created for it. 'canHaveObjectId':
     * Indicates that the object has an Object ID associated with it. 'hasSideEffects': Indicates that the evaluation
     * had side effects. etc.
     */
    std::vector<wxString> attributes;

    /**
     * Visibility of variable. Before introducing additional values, try to use the listed values.
     * Values: 'public', 'private', 'protected', 'internal', 'final', etc.
     */
    wxString visibility;
    ANY_CLASS(VariablePresentationHint);
    JSON_SERIALIZE();
};

/// A Variable is a name/value pair.
/// Optionally a variable can have a 'type' that is shown if space permits or when hovering over the variable's name.
/// An optional 'kind' is used to render additional properties of the variable, e.g. different icons can be used to
/// indicate that a variable is public or private. If the value is structured (has children), a handle is provided to
/// retrieve the children with the VariablesRequest. If the number of named or indexed children is large, the numbers
/// should be returned via the optional 'namedVariables' and 'indexedVariables' attributes. The client can use this
/// optional information to present the children in a paged UI and fetch them in chunks.
struct WXDLLIMPEXP_DAP Variable : public Any {
    wxString name;
    wxString value;
    wxString type;
    /**
     * If variablesReference is > 0, the variable is structured and its children can be retrieved by passing
     * variablesReference to the VariablesRequest.
     */
    int variablesReference = 0;
    VariablePresentationHint presentationHint;
    ANY_CLASS(Variable);
    JSON_SERIALIZE();
};

/// Arguments for 'scopes' request.
struct WXDLLIMPEXP_DAP ScopesArguments : public Any {
    /**
     * Retrieve the scopes for this stackframe.
     */
    int frameId = 0;
    ANY_CLASS(ScopesArguments);
    JSON_SERIALIZE();
};

/// The request returns the variable scopes for a given stackframe ID.
struct WXDLLIMPEXP_DAP ScopesRequest : public Request {
    ScopesArguments arguments;
    REQUEST_CLASS(ScopesRequest, "scopes");
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP Scope : public Any {
    wxString name;
    int variablesReference = 0;
    bool expensive = false;
    Scope(const wxString& n, int varRef)
        : name(n)
        , variablesReference(varRef)
    {
    }

    ANY_CLASS(Scope);
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP ScopesResponse : public Response {
    std::vector<Scope> scopes;
    // extension to the protocol: holds the ID of the thread that owns the scopes
    int refId = wxNOT_FOUND;
    RESPONSE_CLASS(ScopesResponse, "scopes");
    JSON_SERIALIZE();
};

/// Arguments for 'stackTrace' request.
struct WXDLLIMPEXP_DAP StackTraceArguments : public Any {
    /**
     * Retrieve the stacktrace for this thread.
     */
    int threadId = 0;
    /**
     * The index of the first frame to return; if omitted frames start at 0.
     */
    int startFrame = 0;
    /**
     * The maximum number of frames to return. If levels is not specified or 0, all frames are returned.
     */
    int levels = 0;
    ANY_CLASS(StackTraceArguments);
    JSON_SERIALIZE();
};

/// The request returns a stacktrace from the current execution state.
struct WXDLLIMPEXP_DAP StackTraceRequest : public Request {
    StackTraceArguments arguments;
    REQUEST_CLASS(StackTraceRequest, "stackTrace");
    JSON_SERIALIZE();
};

/// Response to 'stackTrace' request.
struct WXDLLIMPEXP_DAP StackTraceResponse : public Response {
    std::vector<StackFrame> stackFrames;
    // extension to the protocol: holds the ID of the thread that owns the frames
    int refId = wxNOT_FOUND;
    RESPONSE_CLASS(StackTraceResponse, "stackTrace");
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP ValueFormat : public Any {
    /**
     * Display the value in hex.
     */
    bool hex = false;
    ANY_CLASS(ValueFormat);
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP VariablesArguments : public Any {
    /**
     * The Variable reference.
     */
    int variablesReference = 0;
    ValueFormat format;
    int count = 0;
    ANY_CLASS(VariablesArguments);
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP VariablesRequest : public Request {
    VariablesArguments arguments;
    REQUEST_CLASS(VariablesRequest, "variables");
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP VariablesResponse : public Response {
    /**
     * All (or a range) of variables for the given variable reference.
     */
    std::vector<Variable> variables;
    // extension to the protocol: holds the parent of these variables
    int refId = wxNOT_FOUND;

    // extension to the protocol: the context for this variable
    EvaluateContext context = EvaluateContext::VARIABLES;

    RESPONSE_CLASS(VariablesResponse, "variables");
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP PauseArguments : public Any {
    int threadId = 0;
    ANY_CLASS(PauseArguments);
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP RunInTerminalRequestArguments : public Any {
    /**
     * What kind of terminal to launch.
     * Values: 'integrated', 'external', etc.
     */
    wxString kind;
    /**
     * Optional title of the terminal.
     */
    wxString title;
    /**
     * List of arguments. The first argument is the command to run.
     */
    std::vector<wxString> args;
    ANY_CLASS(RunInTerminalRequestArguments);
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP RunInTerminalRequest : public Request {
    RunInTerminalRequestArguments arguments;
    REQUEST_CLASS(RunInTerminalRequest, "runInTerminal");
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP RunInTerminalResponse : public Response {
    int processId = wxNOT_FOUND;
    RESPONSE_CLASS(RunInTerminalResponse, "runInTerminal");
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP PauseRequest : public Request {
    PauseArguments arguments;
    REQUEST_CLASS(PauseRequest, "pause");
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP PauseResponse : public EmptyAckResponse {
    RESPONSE_CLASS(PauseResponse, "pause");
};

/// Arguments for SourceRequest
struct WXDLLIMPEXP_DAP SourceArguments : public Any {
    /**
     * Specifies the source content to load. Either source.path or
     * source.sourceReference must be specified.
     */
    dap::Source source;
    /**
     * The reference to the source. This is the same as source.sourceReference.
     * This is provided for backward compatibility since old backends do not
     * understand the 'source' attribute.
     */
    int sourceReference = 0;
    ANY_CLASS(SourceArguments);
    JSON_SERIALIZE();
};

// source request
struct WXDLLIMPEXP_DAP SourceRequest : public Request {
    SourceArguments arguments;
    REQUEST_CLASS(SourceRequest, "source");
    JSON_SERIALIZE();
};

// source response
struct WXDLLIMPEXP_DAP SourceResponse : public Response {
    /**
     * Content of the source reference.
     */
    wxString content;
    /**
     * Optional content type (mime type) of the source.
     */
    wxString mimeType;
    RESPONSE_CLASS(SourceResponse, "source");
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP EvaluateArguments : public Any {
    /**
     * The expression to evaluate.
     */
    wxString expression;

    /**
     * Evaluate the expression in the scope of this stack frame. If not specified,
     * the expression is evaluated in the global scope.
     */
    int frameId = wxNOT_FOUND;

    /**
     * The context in which the evaluate request is used.
     * Values:
     * 'variables': evaluate is called from a variables view context.
     * 'watch': evaluate is called from a watch view context.
     * 'repl': evaluate is called from a REPL context.
     * 'hover': evaluate is called to generate the debug hover contents.
     * This value should only be used if the capability
     * 'supportsEvaluateForHovers' is true.
     * 'clipboard': evaluate is called to generate clipboard contents.
     * This value should only be used if the capability 'supportsClipboardContext'
     * is true.
     * etc.
     */
    wxString context = "hover";

    /**
     * Specifies details on how to format the result.
     * The attribute is only honored by a debug adapter if the capability
     * 'supportsValueFormattingOptions' is true.
     */
    ValueFormat format;
    ANY_CLASS(EvaluateArguments);
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP EvaluateRequest : public Request {
    EvaluateArguments arguments;
    REQUEST_CLASS(EvaluateRequest, "evaluate");
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP EvaluateResponse : public Response {
    /**
     * The result of the evaluate request.
     */
    wxString result;

    /**
     * The optional type of the evaluate result.
     * This attribute should only be returned by a debug adapter if the client
     * has passed the value true for the 'supportsVariableType' capability of
     * the 'initialize' request.
     */
    wxString type;

    /**
     * The number of named child variables.
     * The client can use this optional information to present the variables in
     * a paged UI and fetch them in chunks.
     * The value should be less than or equal to 2147483647 (2^31-1).
     */
    int variablesReference = 0;

    RESPONSE_CLASS(EvaluateResponse, "evaluate");
    JSON_SERIALIZE();
};

struct WXDLLIMPEXP_DAP DebugpyWaitingForServerEvent : public Event {
    wxString host;
    int port;
    EVENT_CLASS(DebugpyWaitingForServerEvent, wxEmptyString);
    JSON_SERIALIZE();
};
}; // namespace dap

namespace std
{
// custom hash functions for some of the dap structures
template <>
struct hash<dap::Breakpoint> {
    std::size_t operator()(const dap::Breakpoint& b) const
    {
        wxString s;
        s << b.source.path << b.source.name << b.line;
        return hash<std::wstring>{}(s.ToStdWstring());
    }
};
template <>
struct hash<dap::SourceBreakpoint> {
    std::size_t operator()(const dap::SourceBreakpoint& b) const { return hash<size_t>{}(b.line); }
};

template <>
struct hash<dap::FunctionBreakpoint> {
    std::size_t operator()(const dap::FunctionBreakpoint& b) const
    {
        return hash<std::wstring>{}(b.name.ToStdWstring());
    }
};
} // namespace std
#endif // PROTOCOLMESSAGE_HPP
