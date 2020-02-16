#include "SocketBase.hpp"
#include "dap.hpp"

#define CREATE_JSON() JSON json = JSON::CreateObject()
#define REQUEST_TO() JSON json = Request::To()
#define RESPONSE_TO() JSON json = Response::To()
#define PROTOCOL_MSG_TO() JSON json = ProtocolMessage::To()
#define EVENT_TO() JSON json = Event::To()
#define ADD_PROP(obj) json.Add(#obj, obj)

#define REQUEST_FROM() Request::From(json)
#define RESPONSE_FROM() Response::From(json)
#define EVENT_FROM() Event::From(json)
#define PROTOCOL_MSG_FROM() ProtocolMessage::From(json)
#define READ_OBJ(obj) obj.From(json[#obj])
#define ADD_OBJ(obj) json.AddObject(#obj, obj.To())
#define GET_PROP(prop, Type) prop = json[#prop].Get##Type()
#define ADD_BODY() JSON body = json.AddObject("body")
#define ADD_BODY_PROP(prop) body.Add(#prop, prop)

#define ADD_ARRAY(Parent, Name) JSON arr = Parent.AddArray(Name);

#define READ_BODY() JSON body = json["body"]

#define GET_BODY_PROP(prop, Type) prop = body[#prop].Get##Type()

namespace dap
{
void Initialize()
{
    REGISTER_CLASS(CancelRequest);
    REGISTER_CLASS(InitializeRequest);
    REGISTER_CLASS(BreakpointLocationsRequest);
    REGISTER_CLASS(ConfigurationDoneRequest);
    REGISTER_CLASS(LaunchRequest);
    REGISTER_CLASS(DisconnectRequest);
    REGISTER_CLASS(SetBreakpointsRequest);
    REGISTER_CLASS(ContinueRequest);
    REGISTER_CLASS(NextRequest);

    REGISTER_CLASS(InitializedEvent);
    REGISTER_CLASS(StoppedEvent);
    REGISTER_CLASS(ContinuedEvent);
    REGISTER_CLASS(ExitedEvent);
    REGISTER_CLASS(TerminatedEvent);
    REGISTER_CLASS(ThreadEvent);
    REGISTER_CLASS(OutputEvent);
    REGISTER_CLASS(BreakpointEvent);
    REGISTER_CLASS(ProcessEvent);

    REGISTER_CLASS(InitializeResponse);
    REGISTER_CLASS(CancelResponse);
    REGISTER_CLASS(ConfigurationDoneResponse);
    REGISTER_CLASS(LaunchResponse);
    REGISTER_CLASS(DisconnectResponse);
    REGISTER_CLASS(BreakpointLocationsResponse);
    REGISTER_CLASS(SetBreakpointsResponse);
    REGISTER_CLASS(ContinueResponse);
    REGISTER_CLASS(NextResponse);
    // Needed for windows socket library
    SocketBase::Initialize();
}
ObjGenerator& ObjGenerator::Get()
{
    static ObjGenerator generator;
    return generator;
}

ProtocolMessage::Ptr_t ObjGenerator::New(const string& type, const string& name)
{
    if(type == "response") {
        return New(name, m_responses);
    } else if(type == "request") {
        return New(name, m_requests);
    } else if(type == "event") {
        return New(name, m_events);
    } else {
        return nullptr;
    }
}

string dap::ProtocolMessage::ToString() const
{
    JSON json = To();
    return json.ToString();
}

void ObjGenerator::RegisterResponse(const string& name, onNewObject func)
{
    // register new response class
    m_responses.insert({ name, func });
}

void ObjGenerator::RegisterEvent(const string& name, onNewObject func)
{
    // register new event class
    m_events.insert({ name, func });
}

void ObjGenerator::RegisterRequest(const string& name, onNewObject func)
{
    // register new request class
    m_requests.insert({ name, func });
}

ProtocolMessage::Ptr_t ObjGenerator::New(const string& name, const unordered_map<string, onNewObject>& pool)
{
    const auto& iter = pool.find(name);
    if(iter == pool.end()) {
        return nullptr;
    }
    return iter->second();
}

///=====================================================================================================
///=====================================================================================================
///=====================================================================================================

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON ProtocolMessage::To() const
{
    CREATE_JSON();
    ADD_PROP(seq);
    ADD_PROP(type);
    return json;
}

void ProtocolMessage::From(const JSON& json)
{
    GET_PROP(seq, Number);
    GET_PROP(type, String);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------
Request::~Request() {}

JSON Request::To() const
{
    PROTOCOL_MSG_TO();
    ADD_PROP(command);
    return json;
}

void Request::From(const JSON& json)
{
    PROTOCOL_MSG_FROM();
    GET_PROP(command, String);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON CancelRequest::To() const
{
    JSON json = Request::To();
    JSON arguments = json.AddObject("arguments");
    arguments.Add("requestId", requestId);
    return json;
}

void CancelRequest::From(const JSON& json)
{
    Request::From(json);
    if(json["arguments"].IsOK()) {
        requestId = json["arguments"].GetInteger();
    }
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------
Event::~Event() {}

JSON Event::To() const
{
    JSON json = ProtocolMessage::To();
    json.Add("event", event);
    return json;
}

void Event::From(const JSON& json)
{
    ProtocolMessage::From(json);
    event = json["event"].GetString();
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------
Response::~Response() {}
JSON Response::To() const
{
    JSON json = ProtocolMessage::To();
    ADD_PROP(request_seq);
    ADD_PROP(success);
    ADD_PROP(message);
    ADD_PROP(command);
    return json;
}

void Response::From(const JSON& json)
{
    ProtocolMessage::From(json);
    GET_PROP(request_seq, Integer);
    GET_PROP(success, Bool);
    GET_PROP(message, String);
    GET_PROP(command, String);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON InitializedEvent::To() const
{
    JSON json = Event::To();
    return json;
}

void InitializedEvent::From(const JSON& json) { Event::From(json); }

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON StoppedEvent::To() const
{
    EVENT_TO();
    ADD_BODY();

    body.Add("reason", reason);
    body.Add("text", text);
    return json;
}

void StoppedEvent::From(const JSON& json)
{
    Event::From(json);
    JSON body = json["body"];
    reason = body["reason"].GetString();
    text = body["text"].GetString();
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON ContinuedEvent::To() const
{
    JSON json = Event::To();
    JSON body = json.AddObject("body");
    body.Add("threadId", threadId);
    body.Add("allThreadsContinued", allThreadsContinued);
    return json;
}

void ContinuedEvent::From(const JSON& json)
{
    Event::From(json);
    JSON body = json["body"];
    threadId = body["threadId"].GetInteger();
    allThreadsContinued = body["allThreadsContinued"].GetBool(false);
}
// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON ExitedEvent::To() const
{
    JSON json = Event::To();
    JSON body = json.AddObject("body");
    body.Add("exitCode", exitCode);
    return json;
}

void ExitedEvent::From(const JSON& json)
{
    Event::From(json);
    JSON body = json["body"];
    exitCode = body["exitCode"].GetInteger();
}
// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON TerminatedEvent::To() const
{
    JSON json = Event::To();
    return json;
}

void TerminatedEvent::From(const JSON& json) { Event::From(json); }

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON ThreadEvent::To() const
{
    JSON json = Event::To();
    JSON body = json.AddObject("body");
    body.Add("reason", reason);
    body.Add("threadId", threadId);
    return json;
}

void ThreadEvent::From(const JSON& json)
{
    Event::From(json);
    JSON body = json["body"];
    reason = body["reason"].GetString();
    threadId = body["threadId"].GetInteger();
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON OutputEvent::To() const
{
    JSON json = Event::To();
    JSON body = json.AddObject("body");
    body.Add("category", category);
    body.Add("output", output);
    return json;
}

void OutputEvent::From(const JSON& json)
{
    Event::From(json);
    JSON body = json["body"];
    category = body["category"].GetString();
    output = body["output"].GetString();
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON Source::To() const
{
    CREATE_JSON();
    json.Add("name", this->name);
    ADD_PROP(path);
    return json;
}

void Source::From(const JSON& json)
{
    GET_PROP(name, String);
    GET_PROP(path, String);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON StackFrame::To() const
{
    CREATE_JSON();
    ADD_PROP(name);
    ADD_PROP(number);
    ADD_PROP(line);
    ADD_OBJ(source);
    return json;
}

void StackFrame::From(const JSON& json)
{
    GET_PROP(name, String);
    GET_PROP(number, Integer);
    GET_PROP(line, Integer);
    READ_OBJ(source);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON Breakpoint::To() const
{
    CREATE_JSON();
    ADD_PROP(id);
    ADD_PROP(verified);
    ADD_PROP(message);
    ADD_PROP(line);
    ADD_PROP(column);
    ADD_PROP(endLine);
    ADD_PROP(endColumn);
    ADD_OBJ(source);
    return json;
}

void Breakpoint::From(const JSON& json)
{
    GET_PROP(id, Integer);
    GET_PROP(verified, Bool);
    GET_PROP(message, String);
    GET_PROP(line, Integer);
    GET_PROP(column, Integer);
    GET_PROP(endLine, Integer);
    GET_PROP(endColumn, Integer);
    READ_OBJ(source);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON BreakpointEvent::To() const
{
    JSON json = Event::To();
    JSON body = json.AddObject("body");
    body.Add("reason", reason);
    body.AddObject("breakpoint", breakpoint.To());
    return json;
}

void BreakpointEvent::From(const JSON& json)
{
    Event::From(json);
    JSON body = json["body"];
    reason = body["reason"].GetString();
    breakpoint.From(body["breakpoint"]);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON ProcessEvent::To() const
{
    JSON json = Event::To();
    JSON body = json.AddObject("body");
    body.Add("name", name);
    body.Add("systemProcessId", systemProcessId);
    body.Add("isLocalProcess", isLocalProcess);
    body.Add("startMethod", startMethod);
    body.Add("pointerSize", pointerSize);
    return json;
}

void ProcessEvent::From(const JSON& json)
{
    Event::From(json);
    JSON body = json["body"];
    name = body["name"].GetString();
    systemProcessId = body["systemProcessId"].GetInteger();
    isLocalProcess = body["isLocalProcess"].GetBool(true);
    startMethod = body["startMethod"].GetString();
    pointerSize = body["pointerSize"].GetInteger();
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON InitializeRequestArguments::To() const
{
    JSON json = JSON::CreateObject();
    json.Add("clientID", clientID);
    json.Add("clientName", clientName);
    json.Add("adapterID", adapterID);
    json.Add("locale", locale);
    json.Add("linesStartAt1", linesStartAt1);
    json.Add("columnsStartAt1", columnsStartAt1);
    json.Add("pathFormat", pathFormat);
    return json;
}

void InitializeRequestArguments::From(const JSON& json)
{
    clientID = json["clientID"].GetString();
    clientName = json["clientName"].GetString();
    adapterID = json["adapterID"].GetString();
    locale = json["locale"].GetString();
    linesStartAt1 = json["linesStartAt1"].GetBool();
    columnsStartAt1 = json["columnsStartAt1"].GetBool();
    pathFormat = json["pathFormat"].GetString();
}
// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON InitializeRequest::To() const
{
    JSON json = Request::To();
    json.AddObject("arguments", arguments.To());
    return json;
}

void InitializeRequest::From(const JSON& json)
{
    Request::From(json);
    arguments.From(json["arguments"]);
}
// ----------------------------------------
// ----------------------------------------
// ----------------------------------------
JSON InitializeResponse::To() const
{
    JSON json = Response::To();
    JSON body = json.AddObject("body");
    return json;
}

void InitializeResponse::From(const JSON& json) { Response::From(json); }

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON ConfigurationDoneRequest::To() const
{
    JSON json = Request::To();
    return json;
}

void ConfigurationDoneRequest::From(const JSON& json) { Request::From(json); }

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------
JSON EmptyAckResponse::To() const
{
    JSON json = Response::To();
    return json;
}

void EmptyAckResponse::From(const JSON& json) { Response::From(json); }

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------
JSON LaunchRequestArguments::To() const
{
    CREATE_JSON();
    ADD_PROP(noDebug);
    ADD_PROP(debuggee);
    ADD_PROP(workingDirectory);
    return json;
}

void LaunchRequestArguments::From(const JSON& json)
{
    GET_PROP(noDebug, Bool);
    GET_PROP(debuggee, StringArray);
    GET_PROP(workingDirectory, String);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON LaunchRequest::To() const
{
    JSON json = Request::To();
    json.AddObject("arguments", arguments.To());
    return json;
}

void LaunchRequest::From(const JSON& json)
{
    Request::From(json);
    arguments.From(json["arguments"]);
}
// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON DisconnectRequest::To() const
{
    JSON json = Request::To();
    JSON arguments = json.AddObject("arguments");
    arguments.Add("restart", restart);
    arguments.Add("terminateDebuggee", terminateDebuggee);
    return json;
}

void DisconnectRequest::From(const JSON& json)
{
    Request::From(json);
    JSON arguments = json["arguments"];
    restart = arguments["restart"].GetBool();
    terminateDebuggee = arguments["terminateDebuggee"].GetBool(terminateDebuggee);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON BreakpointLocationsRequest::To() const
{
    JSON json = Request::To();
    json.AddObject("arguments", arguments.To());
    return json;
}

void BreakpointLocationsRequest::From(const JSON& json)
{
    Request::From(json);
    arguments.From(json["arguments"]);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON BreakpointLocationsArguments::To() const
{
    JSON json = JSON::CreateObject();
    json.Add("source", source.To());
    json.Add("line", line);
    json.Add("column", column);
    json.Add("endLine", endLine);
    json.Add("endColumn", endColumn);
    return json;
}

void BreakpointLocationsArguments::From(const JSON& json)
{
    source.From(json["source"]);
    line = json["restart"].GetInteger(line);
    column = json["column"].GetInteger(column);
    column = json["column"].GetInteger(column);
    endColumn = json["endColumn"].GetInteger(endColumn);
}
// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON BreakpointLocation::To() const
{
    JSON json = JSON::CreateObject();
    json.Add("line", line);
    json.Add("column", column);
    json.Add("endLine", endLine);
    json.Add("endColumn", endColumn);
    return json;
}

void BreakpointLocation::From(const JSON& json)
{
    line = json["restart"].GetInteger(line);
    column = json["column"].GetInteger(column);
    column = json["column"].GetInteger(column);
    endColumn = json["endColumn"].GetInteger(endColumn);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON BreakpointLocationsResponse::To() const
{
    RESPONSE_TO();
    ADD_BODY();
    // create arr
    ADD_ARRAY(body, "breakpoints");
    for(const auto& b : breakpoints) {
        arr.Add(b.To());
    }
    return json;
}

void BreakpointLocationsResponse::From(const JSON& json)
{
    Response::From(json);
    JSON body = json["body"];
    JSON arr = body["breakpoints"];
    breakpoints.clear();
    size_t size = arr.GetCount();
    breakpoints.reserve(size);
    for(size_t i = 0; i < size; ++i) {
        BreakpointLocation loc;
        loc.From(arr[i]);
        breakpoints.push_back(loc);
    }
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON SourceBreakpoint::To() const
{
    JSON json = JSON::CreateObject();
    json.Add("line", line);
    json.Add("condition", condition);
    return json;
}

void SourceBreakpoint::From(const JSON& json)
{
    line = json["line"].GetInteger(line);
    condition = json["condition"].GetString(condition);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON SetBreakpointsArguments::To() const
{
    JSON json = JSON::CreateObject();
    json.Add("source", source.To());

    JSON arr = json.AddArray("breakpoints");
    for(const auto& sb : breakpoints) {
        arr.Add(sb.To());
    }
    return json;
}

void SetBreakpointsArguments::From(const JSON& json)
{
    source.From(json["source"]);
    breakpoints.clear();
    JSON arr = json["breakpoints"];
    int size = arr.GetCount();
    for(int i = 0; i < size; ++i) {
        SourceBreakpoint sb;
        sb.From(arr[i]);
        breakpoints.push_back(sb);
    }
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON SetBreakpointsRequest::To() const
{
    REQUEST_TO();
    ADD_OBJ(arguments);
    return json;
}

void SetBreakpointsRequest::From(const JSON& json)
{
    REQUEST_FROM();
    READ_OBJ(arguments);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON ContinueArguments::To() const
{
    CREATE_JSON();
    ADD_PROP(threadId);
    return json;
}

void ContinueArguments::From(const JSON& json) { GET_PROP(threadId, Integer); }

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON NextArguments::To() const
{
    CREATE_JSON();
    ADD_PROP(threadId);
    return json;
}

void NextArguments::From(const JSON& json) { GET_PROP(threadId, Integer); }

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON ContinueRequest::To() const
{
    REQUEST_TO();
    ADD_OBJ(arguments);
    return json;
}

void ContinueRequest::From(const JSON& json)
{
    REQUEST_FROM();
    READ_OBJ(arguments);
}
// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON NextRequest::To() const
{
    REQUEST_TO();
    ADD_OBJ(arguments);
    return json;
}

void NextRequest::From(const JSON& json)
{
    REQUEST_FROM();
    READ_OBJ(arguments);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON ContinueResponse::To() const
{
    RESPONSE_TO();
    ADD_BODY();
    ADD_BODY_PROP(allThreadsContinued);
    return json;
}

void ContinueResponse::From(const JSON& json)
{
    RESPONSE_FROM();
    READ_BODY();
    GET_BODY_PROP(allThreadsContinued, Number);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

JSON SetBreakpointsResponse::To() const
{
    RESPONSE_TO();
    ADD_BODY();
    // create arr
    ADD_ARRAY(body, "breakpoints");
    for(const auto& b : breakpoints) {
        arr.Add(b.To());
    }
    return json;
}

void SetBreakpointsResponse::From(const JSON& json)
{
    RESPONSE_FROM();
    READ_BODY();
    JSON arr = body["breakpoints"];
    breakpoints.clear();
    int size = arr.GetCount();
    for(int i = 0; i < size; ++i) {
        Breakpoint bp;
        bp.From(arr[i]);
        breakpoints.push_back(bp);
    }
}

}; // namespace dap
