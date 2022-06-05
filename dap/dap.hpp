#ifndef PROTOCOLMESSAGE_HPP
#define PROTOCOLMESSAGE_HPP

#include "JSON.hpp"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/// C++ Implementation of Debug Adapter Protocol (DAP)
/// The Debug Adapter Protocol defines the protocol used between an editor or
/// IDE and a debugger or runtime The implementation is based on the
/// specifications described here:
/// https://microsoft.github.io/debug-adapter-protocol/specification
#define JSON_SERIALIZE()      \
    JSON To() const override; \
    void From(const JSON& json) override

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
void Initialize();
// base class representing anything

struct Any {
    Any() {}
    virtual ~Any() {}

    virtual JSON To() const = 0;
    virtual void From(const JSON& json) = 0;

    template <typename T>
    T* As() const
    {
        return dynamic_cast<T*>(const_cast<Any*>(this));
    }
};

enum class eScopes {
    kArguments = 0,
    kLocals = 1,
    kRegisters = 2,
};
struct Event;
struct Request;
struct Response;

/// Base class of requests, responses, and events
struct ProtocolMessage : public Any {
    int seq = -1;
    std::string type;
    typedef shared_ptr<ProtocolMessage> Ptr_t;

    dap::Event* AsEvent() const { return As<dap::Event>(); }
    dap::Request* AsRequest() const { return As<dap::Request>(); }
    dap::Response* AsResponse() const { return As<dap::Response>(); }

    std::string ToString() const;
    ANY_CLASS(ProtocolMessage);
    JSON_SERIALIZE();
};

class ObjGenerator
{
    typedef function<ProtocolMessage::Ptr_t()> onNewObject;
    unordered_map<std::string, onNewObject> m_responses;
    unordered_map<std::string, onNewObject> m_events;
    unordered_map<std::string, onNewObject> m_requests;

protected:
    ProtocolMessage::Ptr_t New(const std::string& name, const unordered_map<std::string, onNewObject>& pool);

public:
    static ObjGenerator& Get();
    /**
     * @brief create new ProtocolMessage.
     * @param type can be one of ["response", "event", "request"] anything else will return nullptr
     * @param name the class name
     * @return
     */
    ProtocolMessage::Ptr_t New(const std::string& type, const std::string& name);

    /**
     * @brief create new ProtocolMessage from raw JSON object
     */
    ProtocolMessage::Ptr_t FromJSON(JSON json);

    void RegisterResponse(const std::string& name, onNewObject func);
    void RegisterEvent(const std::string& name, onNewObject func);
    void RegisterRequest(const std::string& name, onNewObject func);
};

/// A client or debug adapter initiated request
/// ->
struct Request : public ProtocolMessage {
    std::string command;

    Request() { type = "request"; }
    virtual ~Request() = 0; // force to abstract class
    JSON_SERIALIZE();
};

/// The 'cancel' request is used by the frontend to indicate that it is no
/// longer interested in the result produced by a specific request issued
/// earlier.
/// <->
struct CancelRequest : public Request {
    int requestId = -1;
    REQUEST_CLASS(CancelRequest, "cancel");
    JSON_SERIALIZE();
};

/// A debug adapter initiated event
///
struct Event : public ProtocolMessage {
    std::string event;
    Event() { type = "event"; }
    virtual ~Event() = 0; // force to abstract class
    JSON_SERIALIZE();
};

/// Response for a request
/// <-
struct Response : public ProtocolMessage {
    int request_seq = -1;
    bool success = true;
    std::string command;

    /**
     * Contains the raw error in short form if 'success' is false.
     * This raw error might be interpreted by the frontend and is not shown in
     * the UI. Some predefined values exist. Values: 'cancelled': request was
     * cancelled. etc.
     */
    std::string message;

    Response() { type = "response"; }
    virtual ~Response() = 0; // force to abstract class
    JSON_SERIALIZE();
};

/// Response to 'cancel' request. This is just an acknowledgement, so no body
/// field is required.
/// <-
struct CancelResponse : public Response {
    RESPONSE_CLASS(CancelResponse, "cancel");
    JSON To() const override { return Response::To(); }
    void From(const JSON& json) override { Response::From(json); }
};

/// This event indicates that the debug adapter is ready to accept configuration
/// requests (e.g. SetBreakpointsRequest, SetExceptionBreakpointsRequest).
/// <-
struct InitializedEvent : public Event {
    EVENT_CLASS(InitializedEvent, "initialized");
    JSON_SERIALIZE();
};

/// The event indicates that the execution of the debuggee has stopped due to
/// some condition. This can be caused by a break point previously set, a
/// stepping action has completed, by executing a debugger statement etc.
struct StoppedEvent : public Event {
    /**
     * The reason for the event.
     * For backward compatibility this std::string is shown in the UI if the
     * 'description' attribute is missing (but it must not be translated).
     * Values: 'step', 'breakpoint', 'exception', 'pause', 'entry', 'goto',
     * 'function breakpoint', 'data breakpoint', etc.
     */
    std::string reason;
    /**
     * Additional information. E.g. if reason is 'exception', text contains the
     * exception name. This std::string is shown in the UI.
     */
    std::string text;
    EVENT_CLASS(StoppedEvent, "stopped");
    JSON_SERIALIZE();
};

/// The event indicates that the execution of the debuggee has continued.
/// Please note: a debug adapter is not expected to send this event in response
/// to a request that implies that execution continues, e.g. 'launch' or
/// 'continue'.  It is only necessary to send a 'continued' event if there was
/// no previous request that implied this
struct ContinuedEvent : public Event {
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
struct ExitedEvent : public Event {
    int exitCode = 0;
    EVENT_CLASS(ExitedEvent, "exited");
    JSON_SERIALIZE();
};

/// The event indicates that debugging of the debuggee has terminated. This does
/// not mean that the debuggee itself has exited
struct TerminatedEvent : public Event {
    EVENT_CLASS(TerminatedEvent, "terminated");
    JSON_SERIALIZE();
};

/// The event indicates that a thread has started or exited.
struct ThreadEvent : public Event {
    std::string reason;
    int threadId = 0;
    EVENT_CLASS(ThreadEvent, "thread");
    JSON_SERIALIZE();
};

/// The event indicates that a thread has started or exited.
// <-
struct OutputEvent : public Event {
    /**
     * The output category. If not specified, 'console' is assumed.
     * Values: 'console', 'stdout', 'stderr', 'telemetry', etc.
     */
    std::string category;
    std::string output;

    EVENT_CLASS(OutputEvent, "output");
    JSON_SERIALIZE();
};

/// A Source is a descriptor for source code. It is returned from the debug
/// adapter as part of a StackFrame and it is used by clients when specifying
/// breakpoints
struct Source : public Any {
    /**
     * The short name of the source. Every source returned from the debug
     * adapter has a name. When sending a source to the debug adapter this name
     * is optional.
     */
    std::string name;
    /**
     * The path of the source to be shown in the UI. It is only used to locate
     * and load the content of the source if no sourceReference is specified (or
     * its value is 0).
     */
    std::string path;
    ANY_CLASS(Source);
    JSON_SERIALIZE();
};

/// Information about a Breakpoint created in setBreakpoints or
/// setFunctionBreakpoints.
struct Breakpoint : public Any {
    int id = -1;
    bool verified = false;
    std::string message;
    Source source;
    int line = -1;
    int column = -1;
    int endLine = -1;
    int endColumn = -1;
    ANY_CLASS(Breakpoint);
    JSON_SERIALIZE();
};

/// The event indicates that some information about a breakpoint has changed.
// <-
struct BreakpointEvent : public Event {
    /**
     * The output category. If not specified, 'console' is assumed.
     * Values: 'console', 'stdout', 'stderr', 'telemetry', etc.
     */
    std::string reason;
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
struct ProcessEvent : public Event {
    /**
     * The logical name of the process. This is usually the full path to
     * process's executable file. Example: /home/example/myproj/program.exe
     */
    std::string name;
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
    std::string startMethod;
    int pointerSize = PTR_SIZE;
    EVENT_CLASS(ProcessEvent, "process");
    JSON_SERIALIZE();
};

struct InitializeRequestArguments : public Any {
    /**
     * The ID of the (frontend) client using this adapter.
     */
    std::string clientID;
    /**
     * The human readable name of the (frontend) client using this adapter.
     */
    std::string clientName;
    /**
     * The ID of the debug adapter.
     */
    std::string adapterID;
    /**
     * The ISO-639 locale of the (frontend) client using this adapter, e.g.
     * en-US or de-CH.
     */
    std::string locale = "en-US";
    bool linesStartAt1 = true;
    bool columnsStartAt1 = true;
    /**
     * Determines in what format paths are specified. The default is 'path',
     * which is the native format. Values: 'path', 'uri', etc.
     */
    std::string pathFormat = "path";
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
struct InitializeRequest : public Request {
    InitializeRequestArguments arguments;
    REQUEST_CLASS(InitializeRequest, "initialize");
    JSON_SERIALIZE();
};

/// Response to 'initialize' request.
/// <-
struct InitializeResponse : public Response {
    RESPONSE_CLASS(InitializeResponse, "initialize");
    JSON_SERIALIZE();
};

/// The client of the debug protocol must send this request at the end of the
/// sequence of configuration requests (which was started by the 'initialized'
/// event).
/// <->
struct ConfigurationDoneRequest : public Request {
    REQUEST_CLASS(ConfigurationDoneRequest, "configurationDone");
    JSON_SERIALIZE();
};

struct EmptyAckResponse : public Response {
    RESPONSE_CLASS(EmptyAckResponse, "");
    JSON_SERIALIZE();
};

/// Response to 'configurationDone' request. This is just an acknowledgement, so
/// no body field is required.
/// <-
struct ConfigurationDoneResponse : public EmptyAckResponse {
    RESPONSE_CLASS(ConfigurationDoneResponse, "configurationDone");
};

/// Arguments for 'launch' request. Additional attributes are implementation
/// specific.
struct LaunchRequestArguments : public Any {
    /**
     * If noDebug is true the launch request should launch the program without
     * enabling debugging.
     */
    bool noDebug = false;
    /**
     * the program launch. debuggee[0] should contain the executable
     * the other items in the std::vector are passed to the debuggee
     */
    std::vector<std::string> debuggee;
    /*
     * start the debugger from this directory
     */
    std::string workingDirectory = ".";
    ANY_CLASS(LaunchRequestArguments);
    JSON_SERIALIZE();
};

/// The launch request is sent from the client to the debug adapter to start the
/// debuggee with or without debugging (if 'noDebug' is true). Since launching
/// is debugger/runtime specific, the arguments for this request are not part of
/// this specification
struct LaunchRequest : public Request {
    LaunchRequestArguments arguments;
    REQUEST_CLASS(LaunchRequest, "launch");
    JSON_SERIALIZE();
};

/// Response to 'launch' request. This is just an acknowledgement, so no body
/// field is required.
/// <-
struct LaunchResponse : public EmptyAckResponse {
    RESPONSE_CLASS(LaunchResponse, "launch");
};

/// The 'disconnect' request is sent from the client to the debug adapter in
/// order to stop debugging. It asks the debug adapter to disconnect from the
/// debuggee and to terminate the debug adapter. If the debuggee has been
/// started with the 'launch' request, the 'disconnect' request terminates the
/// debuggee. If the 'attach' request was used to connect to the debuggee,
/// 'disconnect' does not terminate the debuggee. This behavior can be
/// controlled with the 'terminateDebuggee' argument (if supported by the debug
/// adapter).
struct DisconnectRequest : public Request {
    bool restart = false;
    bool terminateDebuggee = true;
    REQUEST_CLASS(DisconnectRequest, "disconnect");
    JSON_SERIALIZE();
};

struct DisconnectResponse : public EmptyAckResponse {
    RESPONSE_CLASS(DisconnectResponse, "disconnect");
};

struct BreakpointLocationsArguments : public Any {
    Source source;
    int line = -1;
    int column = -1;
    int endLine = -1;
    int endColumn = -1;
    ANY_CLASS(BreakpointLocationsArguments);
    JSON_SERIALIZE();
};

/// Properties of a breakpoint location returned from the 'breakpointLocations'
/// request.
struct BreakpointLocation : public Any {
    int line = -1;
    int column = -1;
    int endLine = -1;
    int endColumn = -1;
    ANY_CLASS(BreakpointLocation);
    JSON_SERIALIZE();
};

/// Response to 'breakpointLocations' request.
/// Contains possible locations for source breakpoints.
struct BreakpointLocationsResponse : public Response {
    std::vector<BreakpointLocation> breakpoints;
    RESPONSE_CLASS(BreakpointLocationsResponse, "breakpointLocations");
    JSON_SERIALIZE();
};

/// The 'breakpointLocations' request returns all possible locations for source
/// breakpoints in a given range.
struct BreakpointLocationsRequest : public Request {
    BreakpointLocationsArguments arguments;
    REQUEST_CLASS(BreakpointLocationsRequest, "breakpointLocations");
    JSON_SERIALIZE();
};

/// Properties of a breakpoint or logpoint passed to the setBreakpoints request.
struct SourceBreakpoint : public Any {
    int line = -1;
    /**
     * An optional expression for conditional breakpoints.
     */
    std::string condition;
    SourceBreakpoint() {}
    SourceBreakpoint(int line, const std::string& cond)
    {
        this->line = line;
        this->condition = cond;
    }
    JSON_SERIALIZE();
};

/// Properties of a breakpoint or logpoint passed to the setBreakpoints request.
struct SetBreakpointsArguments : public Any {
    Source source;
    std::vector<SourceBreakpoint> breakpoints;
    JSON_SERIALIZE();
};

/// Sets multiple breakpoints for a single source and clears all previous
/// breakpoints in that source. To clear all breakpoint for a source, specify an
/// empty array. When a breakpoint is hit, a 'stopped' event (with reason
/// 'breakpoint') is generated.
struct SetBreakpointsRequest : public Request {
    SetBreakpointsArguments arguments;
    REQUEST_CLASS(SetBreakpointsRequest, "setBreakpoints");
    JSON_SERIALIZE();
};

/// Response to 'setBreakpoints' request.
/// Returned is information about each breakpoint created by this request.
/// This includes the actual code location and whether the breakpoint could be verified.
/// The breakpoints returned are in the same order as the elements of the 'breakpoints'
///(or the deprecated 'lines') array in the arguments
struct SetBreakpointsResponse : public Response {
    std::vector<Breakpoint> breakpoints;
    RESPONSE_CLASS(SetBreakpointsResponse, "setBreakpoints");
    JSON_SERIALIZE();
};

/// Arguments for the continue request
struct ContinueArguments : public Any {
    /**
     * Continue execution for the specified thread (if possible). If the backend
     * cannot continue on a single thread but will continue on all threads, it
     * should set the 'allThreadsContinued' attribute in the response to true.
     */
    int threadId = -1;
    ANY_CLASS(ContinueArguments);
    JSON_SERIALIZE();
};

/// Arguments for the continue request
struct NextArguments : public Any {
    /**
     * Execute 'next' for this thread.
     */
    int threadId = -1;
    ANY_CLASS(NextArguments);
    JSON_SERIALIZE();
};

/// The request starts the debuggee to run again.
struct ContinueRequest : public Request {
    ContinueArguments arguments;
    REQUEST_CLASS(ContinueRequest, "continue");
    JSON_SERIALIZE();
};

/// Response to 'continue' request.
struct ContinueResponse : public Response {
    bool allThreadsContinued = true;
    RESPONSE_CLASS(ContinueResponse, "continue");
    JSON_SERIALIZE();
};

/// The request starts the debuggee to run again.
struct NextRequest : public Request {
    NextArguments arguments;
    REQUEST_CLASS(NextRequest, "next");
    JSON_SERIALIZE();
};

/// Response to 'continue' request.
struct NextResponse : public EmptyAckResponse {
    RESPONSE_CLASS(NextResponse, "next");
};

/// A Stackframe contains the source location
struct StackFrame : public Any {
    /**
     * An identifier for the stack frame. It must be unique across all threads. This id can be used to retrieve the
     * scopes of the frame with the 'scopesRequest' or to restart the execution of a stackframe.
     */
    int number = -1;
    /**
     * The name of the stack frame, typically a method name.
     */
    std::string name;

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
struct ThreadsRequest : public Request {
    REQUEST_CLASS(ThreadsRequest, "threads");
    JSON_SERIALIZE();
};

/// A Thread
struct Thread : public Any {
    /**
     * Unique identifier for the thread.
     */
    int id = -1;

    /**
     * A name of the thread.
     */
    std::string name;
    ANY_CLASS(Thread);
    JSON_SERIALIZE();
};

/// Response to 'threads' request.
struct ThreadsResponse : public Response {
    std::vector<Thread> threads;
    RESPONSE_CLASS(ThreadsResponse, "threads");
    JSON_SERIALIZE();
};

/// Optional properties of a variable that can be used to determine how to render the variable in the UI.
struct VariablePresentationHint : public Any {
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
    std::string kind;
    /**
     * Set of attributes represented as an array of strings. Before introducing additional values, try to use the listed
     * values. Values: 'static': Indicates that the object is static. 'constant': Indicates that the object is a
     * constant. 'readOnly': Indicates that the object is read only. 'rawString': Indicates that the object is a raw
     * std::string. 'hasObjectId': Indicates that the object can have an Object ID created for it. 'canHaveObjectId':
     * Indicates that the object has an Object ID associated with it. 'hasSideEffects': Indicates that the evaluation
     * had side effects. etc.
     */
    std::vector<std::string> attributes;

    /**
     * Visibility of variable. Before introducing additional values, try to use the listed values.
     * Values: 'public', 'private', 'protected', 'internal', 'final', etc.
     */
    std::string visibility;
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
struct Variable : public Any {
    std::string name;
    std::string value;
    std::string type;
    /**
     * If variablesReference is > 0, the variable is structured and its children can be retrieved by passing
     * variablesReference to the VariablesRequest.
     */
    eScopes variablesReference = eScopes::kArguments;
    VariablePresentationHint presentationHint;
    ANY_CLASS(Variable);
    JSON_SERIALIZE();
};

/// Arguments for 'scopes' request.
struct ScopesArguments : public Any {
    /**
     * Retrieve the scopes for this stackframe.
     */
    int frameId = 0;
    ANY_CLASS(ScopesArguments);
    JSON_SERIALIZE();
};

/// The request returns the variable scopes for a given stackframe ID.
struct ScopesRequest : public Request {
    ScopesArguments arguments;
    REQUEST_CLASS(ScopesRequest, "scopes");
    JSON_SERIALIZE();
};

struct Scope : public Any {
    std::string name;
    eScopes variablesReference = eScopes::kArguments;
    bool expensive = false;
    Scope(const std::string& n, eScopes ref)
        : name(n)
        , variablesReference(ref)
    {
    }

    ANY_CLASS(Scope);
    JSON_SERIALIZE();
};

struct ScopesResponse : public Response {
    std::vector<Scope> scopes;
    RESPONSE_CLASS(ScopesResponse, "scopes");
    JSON_SERIALIZE();
};

/// Arguments for 'stackTrace' request.
struct StackTraceArguments : public Any {
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
struct StackTraceRequest : public Request {
    StackTraceArguments arguments;
    REQUEST_CLASS(StackTraceRequest, "stackTrace");
    JSON_SERIALIZE();
};

/// Response to 'stackTrace' request.
struct StackTraceResponse : public Response {
    std::vector<StackFrame> stackFrames;
    RESPONSE_CLASS(StackTraceResponse, "stackTrace");
    JSON_SERIALIZE();
};

struct ValueFormat : public Any {
    /**
     * Display the value in hex.
     */
    bool hex = false;
    ANY_CLASS(ValueFormat);
    JSON_SERIALIZE();
};

struct VariablesArguments : public Any {
    /**
     * The Variable reference.
     */
    eScopes variablesReference = eScopes::kLocals;
    ValueFormat format;
    ANY_CLASS(VariablesArguments);
    JSON_SERIALIZE();
};

struct VariablesRequest : public Request {
    VariablesArguments arguments;
    REQUEST_CLASS(VariablesRequest, "variables");
    JSON_SERIALIZE();
};

struct VariablesResponse : public Response {
    /**
     * All (or a range) of variables for the given variable reference.
     */
    std::vector<Variable> variables;
    RESPONSE_CLASS(VariablesResponse, "variables");
    JSON_SERIALIZE();
};
};     // namespace dap
#endif // PROTOCOLMESSAGE_HPP
