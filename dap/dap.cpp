#include "dap.hpp"

#include "Socket.hpp"

#define CREATE_JSON() Json json = Json::CreateObject()
#define REQUEST_TO() Json json = Request::To()
#define RESPONSE_TO() Json json = Response::To()
#define PROTOCOL_MSG_TO() Json json = ProtocolMessage::To()
#define EVENT_TO() Json json = Event::To()
#define ADD_PROP(obj) json.Add(#obj, obj)

#define REQUEST_FROM() Request::From(json)
#define RESPONSE_FROM() Response::From(json)
#define EVENT_FROM() Event::From(json)
#define PROTOCOL_MSG_FROM() ProtocolMessage::From(json)
#define READ_OBJ(obj) obj.From(json[#obj])
#define ADD_OBJ(obj) json.AddObject(#obj, obj.To())
#define GET_PROP(prop, Type) prop = json[#prop].Get##Type()
#define ADD_BODY() Json body = json.AddObject("body")
#define ADD_BODY_PROP(prop) body.Add(#prop, prop)

#define ADD_ARRAY(Parent, Name) Json arr = Parent.AddArray(Name);

#define READ_BODY() Json body = json["body"]

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
    REGISTER_CLASS(SetFunctionBreakpointsRequest);
    REGISTER_CLASS(ContinueRequest);
    REGISTER_CLASS(NextRequest);
    REGISTER_CLASS(StepInRequest);
    REGISTER_CLASS(StepOutRequest);
    REGISTER_CLASS(ThreadsRequest);
    REGISTER_CLASS(ScopesRequest);
    REGISTER_CLASS(StackTraceRequest);
    REGISTER_CLASS(PauseRequest);
    REGISTER_CLASS(RunInTerminalRequest);
    REGISTER_CLASS(SourceRequest);
    REGISTER_CLASS(EvaluateRequest);

    REGISTER_CLASS(InitializedEvent);
    REGISTER_CLASS(StoppedEvent);
    REGISTER_CLASS(ContinuedEvent);
    REGISTER_CLASS(ExitedEvent);
    REGISTER_CLASS(TerminatedEvent);
    REGISTER_CLASS(ThreadEvent);
    REGISTER_CLASS(OutputEvent);
    REGISTER_CLASS(BreakpointEvent);
    REGISTER_CLASS(ProcessEvent);
    REGISTER_CLASS(ModuleEvent);

    REGISTER_CLASS(InitializeResponse);
    REGISTER_CLASS(CancelResponse);
    REGISTER_CLASS(ConfigurationDoneResponse);
    REGISTER_CLASS(LaunchResponse);
    REGISTER_CLASS(DisconnectResponse);
    REGISTER_CLASS(BreakpointLocationsResponse);
    REGISTER_CLASS(SetBreakpointsResponse);
    REGISTER_CLASS(SetFunctionBreakpointsResponse);
    REGISTER_CLASS(ContinueResponse);
    REGISTER_CLASS(NextResponse);
    REGISTER_CLASS(StepInResponse);
    REGISTER_CLASS(StepOutResponse);
    REGISTER_CLASS(ThreadsResponse);
    REGISTER_CLASS(ScopesResponse);
    REGISTER_CLASS(StackTraceResponse);
    REGISTER_CLASS(VariablesResponse);
    REGISTER_CLASS(PauseResponse);
    REGISTER_CLASS(RunInTerminalResponse);
    REGISTER_CLASS(SourceResponse);
    REGISTER_CLASS(EvaluateResponse);

    // Needed for windows socket library
    Socket::Initialize();
}
ObjGenerator& ObjGenerator::Get()
{
    static ObjGenerator generator;
    return generator;
}

ProtocolMessage::Ptr_t ObjGenerator::New(const wxString& type, const wxString& name)
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

wxString dap::ProtocolMessage::ToString() const
{
    Json json = To();
    return json.ToString(false);
}

void ObjGenerator::RegisterResponse(const wxString& name, onNewObject func)
{
    // register new response class
    m_responses.insert({ name, func });
}

void ObjGenerator::RegisterEvent(const wxString& name, onNewObject func)
{
    // register new event class
    m_events.insert({ name, func });
}

void ObjGenerator::RegisterRequest(const wxString& name, onNewObject func)
{
    // register new request class
    m_requests.insert({ name, func });
}

ProtocolMessage::Ptr_t ObjGenerator::New(const wxString& name, const unordered_map<wxString, onNewObject>& pool)
{
    const auto& iter = pool.find(name);
    if(iter == pool.end()) {
        return nullptr;
    }
    return iter->second();
}

ProtocolMessage::Ptr_t dap::ObjGenerator::FromJSON(Json json)
{
    if(!json.IsOK()) {
        return nullptr;
    }
    wxString type = json["type"].GetString();
    wxString command = (type == "event") ? json["event"].GetString() : json["command"].GetString();
    ProtocolMessage::Ptr_t msg = New(type, command);
    if(!msg) {
        return nullptr;
    }

    msg->From(json);
    return msg;
}

///=====================================================================================================
///=====================================================================================================
///=====================================================================================================

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json ProtocolMessage::To() const
{
    CREATE_JSON();
    ADD_PROP(seq);
    ADD_PROP(type);
    return json;
}

void ProtocolMessage::From(const Json& json)
{
    GET_PROP(seq, Number);
    GET_PROP(type, String);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------
Request::~Request() {}

Json Request::To() const
{
    PROTOCOL_MSG_TO();
    ADD_PROP(command);
    return json;
}

void Request::From(const Json& json)
{
    PROTOCOL_MSG_FROM();
    GET_PROP(command, String);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json CancelRequest::To() const
{
    Json json = Request::To();
    Json arguments = json.AddObject("arguments");
    arguments.Add("requestId", requestId);
    return json;
}

void CancelRequest::From(const Json& json)
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

Json Event::To() const
{
    Json json = ProtocolMessage::To();
    json.Add("event", event);
    return json;
}

void Event::From(const Json& json)
{
    ProtocolMessage::From(json);
    event = json["event"].GetString();
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------
Response::~Response() {}
Json Response::To() const
{
    Json json = ProtocolMessage::To();
    ADD_PROP(request_seq);
    ADD_PROP(success);
    ADD_PROP(message);
    ADD_PROP(command);
    return json;
}

void Response::From(const Json& json)
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

Json InitializedEvent::To() const
{
    Json json = Event::To();
    return json;
}

void InitializedEvent::From(const Json& json) { Event::From(json); }

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json StoppedEvent::To() const
{
    EVENT_TO();
    ADD_BODY();

    body.Add("reason", reason);
    body.Add("text", text);
    body.Add("description", description);
    body.Add("allThreadsStopped", allThreadsStopped);
    body.Add("threadId", threadId);
    return json;
}

void StoppedEvent::From(const Json& json)
{
    Event::From(json);
    Json body = json["body"];
    reason = body["reason"].GetString();
    text = body["text"].GetString();
    description = body["description"].GetString();
    allThreadsStopped = body["allThreadsStopped"].GetBool();
    threadId = body["threadId"].GetInteger(wxNOT_FOUND);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json ContinuedEvent::To() const
{
    Json json = Event::To();
    Json body = json.AddObject("body");
    body.Add("threadId", threadId);
    body.Add("allThreadsContinued", allThreadsContinued);
    return json;
}

void ContinuedEvent::From(const Json& json)
{
    Event::From(json);
    Json body = json["body"];
    threadId = body["threadId"].GetInteger();
    allThreadsContinued = body["allThreadsContinued"].GetBool(false);
}
// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json ExitedEvent::To() const
{
    Json json = Event::To();
    Json body = json.AddObject("body");
    body.Add("exitCode", exitCode);
    return json;
}

void ExitedEvent::From(const Json& json)
{
    Event::From(json);
    Json body = json["body"];
    exitCode = body["exitCode"].GetInteger();
}
// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json TerminatedEvent::To() const
{
    Json json = Event::To();
    return json;
}

void TerminatedEvent::From(const Json& json) { Event::From(json); }

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json ThreadEvent::To() const
{
    Json json = Event::To();
    Json body = json.AddObject("body");
    body.Add("reason", reason);
    body.Add("threadId", threadId);
    return json;
}

void ThreadEvent::From(const Json& json)
{
    Event::From(json);
    Json body = json["body"];
    reason = body["reason"].GetString();
    threadId = body["threadId"].GetInteger();
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json OutputEvent::To() const
{
    Json json = Event::To();
    Json body = json.AddObject("body");
    body.Add("category", category);
    body.Add("output", output);
    return json;
}

void OutputEvent::From(const Json& json)
{
    Event::From(json);
    Json body = json["body"];
    category = body["category"].GetString();
    output = body["output"].GetString();
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json Source::To() const
{
    CREATE_JSON();
    ADD_PROP(name);

    // serialise these properties only if they contain values
    if(!path.empty()) {
        ADD_PROP(path);
    }

    if(sourceReference > 0) {
        ADD_PROP(sourceReference);
    }
    return json;
}

void Source::From(const Json& json)
{
    GET_PROP(name, String);
    GET_PROP(path, String);
    sourceReference = json["sourceReference"].GetNumber(0);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json StackFrame::To() const
{
    CREATE_JSON();
    ADD_PROP(name);
    ADD_PROP(id);
    ADD_PROP(line);
    ADD_OBJ(source);
    return json;
}

void StackFrame::From(const Json& json)
{
    GET_PROP(name, String);
    GET_PROP(id, Integer);
    GET_PROP(line, Integer);
    READ_OBJ(source);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json Breakpoint::To() const
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

void Breakpoint::From(const Json& json)
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

bool Breakpoint::operator==(const Breakpoint& other) const
{
    return (!source.path.empty() && source.path == other.source.path && line == other.line) ||
           (!source.name.empty() && source.name == other.source.name) ||
           (source.sourceReference == other.source.sourceReference);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json BreakpointEvent::To() const
{
    Json json = Event::To();
    Json body = json.AddObject("body");
    body.Add("reason", reason);
    body.AddObject("breakpoint", breakpoint.To());
    return json;
}

void BreakpointEvent::From(const Json& json)
{
    Event::From(json);
    Json body = json["body"];
    reason = body["reason"].GetString();
    breakpoint.From(body["breakpoint"]);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json ProcessEvent::To() const
{
    Json json = Event::To();
    Json body = json.AddObject("body");
    body.Add("name", name);
    body.Add("systemProcessId", systemProcessId);
    body.Add("isLocalProcess", isLocalProcess);
    body.Add("startMethod", startMethod);
    body.Add("pointerSize", pointerSize);
    return json;
}

void ProcessEvent::From(const Json& json)
{
    Event::From(json);
    Json body = json["body"];
    name = body["name"].GetString();
    systemProcessId = body["systemProcessId"].GetInteger();
    isLocalProcess = body["isLocalProcess"].GetBool(true);
    startMethod = body["startMethod"].GetString();
    pointerSize = body["pointerSize"].GetInteger();
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json InitializeRequestArguments::To() const
{
    Json json = Json::CreateObject();
    json.Add("clientID", clientID);
    json.Add("clientName", clientName);
    json.Add("adapterID", adapterID);
    json.Add("locale", locale);
    json.Add("linesStartAt1", linesStartAt1);
    json.Add("columnsStartAt1", columnsStartAt1);
    json.Add("pathFormat", pathFormat);
    json.Add("supportsInvalidatedEvent", supportsInvalidatedEvent);
    return json;
}

void InitializeRequestArguments::From(const Json& json)
{
    clientID = json["clientID"].GetString();
    clientName = json["clientName"].GetString();
    adapterID = json["adapterID"].GetString();
    locale = json["locale"].GetString();
    linesStartAt1 = json["linesStartAt1"].GetBool();
    columnsStartAt1 = json["columnsStartAt1"].GetBool();
    pathFormat = json["pathFormat"].GetString();
    supportsInvalidatedEvent = json["supportsInvalidatedEvent"].GetBool();
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json InitializeRequest::To() const
{
    Json json = Request::To();
    json.AddObject("arguments", arguments.To());
    return json;
}

void InitializeRequest::From(const Json& json)
{
    Request::From(json);
    arguments.From(json["arguments"]);
}
// ----------------------------------------
// ----------------------------------------
// ----------------------------------------
Json InitializeResponse::To() const
{
    Json json = Response::To();
    Json body = json.AddObject("body");
    return json;
}

void InitializeResponse::From(const Json& json) { Response::From(json); }

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json ConfigurationDoneRequest::To() const
{
    Json json = Request::To();
    return json;
}

void ConfigurationDoneRequest::From(const Json& json) { Request::From(json); }

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------
Json EmptyAckResponse::To() const
{
    Json json = Response::To();
    return json;
}

void EmptyAckResponse::From(const Json& json) { Response::From(json); }

// dap::Environment
Json Environment::To() const
{
    switch(format) {
    case EnvFormat::DICTIONARY: {
        auto env_dict = Json::CreateObject();
        for(const auto& vt : vars) {
            env_dict.Add(vt.first, vt.second);
        }
        return env_dict;
    } break;
    case EnvFormat::LIST: {
        auto env_arr = Json::CreateArray();
        for(const auto& vt : vars) {
            env_arr.Add(vt.first + "=" + vt.second);
        }
        return env_arr;
    } break;
    case EnvFormat::NONE:
        return {};
    }
    return {};
}

void Environment::From(const Json& json)
{
    vars.clear();
    // From() is called when in server mode, i.e. we get to choose which format we accept
    // and we choose to support the LIST format
    if(!json.IsOK() || !json.IsArray()) {
        return;
    }

    size_t count = json.GetCount();
    for(size_t i = 0; i < count; ++i) {
        wxString str = json[i].GetString();
        if(str.Index('=') == wxNOT_FOUND)
            continue;
        wxString key = str.BeforeFirst('=');
        wxString value = str.AfterFirst('=');
        vars.insert({ key, value });
    }
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------
Json LaunchRequestArguments::To() const
{
    CREATE_JSON();
    ADD_PROP(noDebug);
    ADD_PROP(program);
    ADD_PROP(args);
    ADD_PROP(cwd);
    auto env_obj = env.To();
    if(env_obj.IsOK()) {
        json.Add("env", env.To());
    }
    return json;
}

void LaunchRequestArguments::From(const Json& json)
{
    GET_PROP(noDebug, Bool);
    GET_PROP(program, String);
    GET_PROP(args, StringArray);
    GET_PROP(cwd, String);
    env.From(json["env"]);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json LaunchRequest::To() const
{
    Json json = Request::To();
    json.AddObject("arguments", arguments.To());
    return json;
}

void LaunchRequest::From(const Json& json)
{
    Request::From(json);
    arguments.From(json["arguments"]);
}
// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json DisconnectRequest::To() const
{
    Json json = Request::To();
    Json arguments = json.AddObject("arguments");
    arguments.Add("restart", restart);
    arguments.Add("terminateDebuggee", terminateDebuggee);
    return json;
}

void DisconnectRequest::From(const Json& json)
{
    Request::From(json);
    Json arguments = json["arguments"];
    restart = arguments["restart"].GetBool();
    terminateDebuggee = arguments["terminateDebuggee"].GetBool(terminateDebuggee);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json BreakpointLocationsRequest::To() const
{
    Json json = Request::To();
    json.AddObject("arguments", arguments.To());
    return json;
}

void BreakpointLocationsRequest::From(const Json& json)
{
    Request::From(json);
    arguments.From(json["arguments"]);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json BreakpointLocationsArguments::To() const
{
    Json json = Json::CreateObject();
    json.Add("source", source.To());
    json.Add("line", line);
    json.Add("column", column);
    json.Add("endLine", endLine);
    json.Add("endColumn", endColumn);
    return json;
}

void BreakpointLocationsArguments::From(const Json& json)
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

Json StepArguments::To() const
{
    Json json = Json::CreateObject();
    json.Add("threadId", threadId);
    json.Add("singleThread", singleThread);
    json.Add("granularity", granularity);
    return json;
}

void StepArguments::From(const Json& json)
{
    threadId = json["threadId"].GetInteger(wxNOT_FOUND);
    singleThread = json["singleThread"].GetBool(false);
    granularity = json["granularity"].GetString(granularity);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json BreakpointLocation::To() const
{
    Json json = Json::CreateObject();
    json.Add("line", line);
    json.Add("column", column);
    json.Add("endLine", endLine);
    json.Add("endColumn", endColumn);
    return json;
}

void BreakpointLocation::From(const Json& json)
{
    line = json["restart"].GetInteger(line);
    column = json["column"].GetInteger(column);
    column = json["column"].GetInteger(column);
    endColumn = json["endColumn"].GetInteger(endColumn);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json Thread::To() const
{
    Json json = Json::CreateObject();
    ADD_PROP(id);
    ADD_PROP(name);
    return json;
}

void Thread::From(const Json& json)
{
    id = json["id"].GetInteger(id);
    name = json["name"].GetString();
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json BreakpointLocationsResponse::To() const
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

void BreakpointLocationsResponse::From(const Json& json)
{
    Response::From(json);
    Json body = json["body"];
    Json arr = body["breakpoints"];
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

Json SourceBreakpoint::To() const
{
    Json json = Json::CreateObject();
    json.Add("line", line);
    json.Add("condition", condition);
    return json;
}

void SourceBreakpoint::From(const Json& json)
{
    line = json["line"].GetInteger(line);
    condition = json["condition"].GetString(condition);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json FunctionBreakpoint::To() const
{
    Json json = Json::CreateObject();
    json.Add("name", name);
    json.Add("condition", condition);
    return json;
}

void FunctionBreakpoint::From(const Json& json)
{
    name = json["name"].GetString(name);
    condition = json["condition"].GetString(condition);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json SetBreakpointsArguments::To() const
{
    Json json = Json::CreateObject();
    json.Add("source", source.To());

    Json arr = json.AddArray("breakpoints");
    for(const auto& sb : breakpoints) {
        arr.Add(sb.To());
    }
    return json;
}

void SetBreakpointsArguments::From(const Json& json)
{
    source.From(json["source"]);
    breakpoints.clear();
    Json arr = json["breakpoints"];
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

Json SetFunctionBreakpointsArguments::To() const
{
    Json json = Json::CreateObject();
    Json arr = json.AddArray("breakpoints");
    for(const auto& sb : breakpoints) {
        arr.Add(sb.To());
    }
    return json;
}

void SetFunctionBreakpointsArguments::From(const Json& json)
{
    breakpoints.clear();
    Json arr = json["breakpoints"];
    int size = arr.GetCount();
    for(int i = 0; i < size; ++i) {
        FunctionBreakpoint fb;
        fb.From(arr[i]);
        breakpoints.push_back(fb);
    }
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json SetBreakpointsRequest::To() const
{
    REQUEST_TO();
    ADD_OBJ(arguments);
    return json;
}

void SetBreakpointsRequest::From(const Json& json)
{
    REQUEST_FROM();
    READ_OBJ(arguments);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json SetFunctionBreakpointsRequest::To() const
{
    REQUEST_TO();
    ADD_OBJ(arguments);
    return json;
}

void SetFunctionBreakpointsRequest::From(const Json& json)
{
    REQUEST_FROM();
    READ_OBJ(arguments);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json ContinueArguments::To() const
{
    CREATE_JSON();
    ADD_PROP(threadId);
    return json;
}

void ContinueArguments::From(const Json& json) { GET_PROP(threadId, Integer); }

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json NextArguments::To() const
{
    CREATE_JSON();
    ADD_PROP(threadId);
    ADD_PROP(granularity);
    return json;
}

void NextArguments::From(const Json& json)
{
    GET_PROP(threadId, Integer);
    GET_PROP(granularity, String);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json ContinueRequest::To() const
{
    REQUEST_TO();
    ADD_OBJ(arguments);
    return json;
}

void ContinueRequest::From(const Json& json)
{
    REQUEST_FROM();
    READ_OBJ(arguments);
}
// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json StepRequest::To() const
{
    REQUEST_TO();
    ADD_OBJ(arguments);
    return json;
}

void StepRequest::From(const Json& json)
{
    REQUEST_FROM();
    READ_OBJ(arguments);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json NextRequest::To() const
{
    REQUEST_TO();
    ADD_OBJ(arguments);
    return json;
}

void NextRequest::From(const Json& json)
{
    REQUEST_FROM();
    READ_OBJ(arguments);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json ThreadsRequest::To() const
{
    REQUEST_TO();
    return json;
}

void ThreadsRequest::From(const Json& json) { REQUEST_FROM(); }

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json ContinueResponse::To() const
{
    RESPONSE_TO();
    ADD_BODY();
    ADD_BODY_PROP(allThreadsContinued);
    return json;
}

void ContinueResponse::From(const Json& json)
{
    RESPONSE_FROM();
    READ_BODY();
    GET_BODY_PROP(allThreadsContinued, Number);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json SetBreakpointsResponse::To() const
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

void SetBreakpointsResponse::From(const Json& json)
{
    RESPONSE_FROM();
    READ_BODY();
    Json arr = body["breakpoints"];
    breakpoints.clear();
    int size = arr.GetCount();
    for(int i = 0; i < size; ++i) {
        Breakpoint bp;
        bp.From(arr[i]);
        breakpoints.push_back(bp);
    }
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json ThreadsResponse::To() const
{
    RESPONSE_TO();
    ADD_BODY();
    // create arr
    ADD_ARRAY(body, "threads");
    for(const auto& thr : threads) {
        arr.Add(thr.To());
    }
    return json;
}

void ThreadsResponse::From(const Json& json)
{
    RESPONSE_FROM();
    READ_BODY();
    Json arr = body["threads"];
    threads.clear();
    int size = arr.GetCount();
    threads.reserve(size);
    for(int i = 0; i < size; ++i) {
        Thread thr;
        thr.From(arr[i]);
        threads.push_back(thr);
    }
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json VariablePresentationHint::To() const
{
    Json json = Json::CreateObject();
    json.Add("kind", kind);
    json.Add("visibility", visibility);
    json.Add("attributes", attributes);
    return json;
}

void VariablePresentationHint::From(const Json& json)
{
    kind = json["kind"].GetString();
    visibility = json["visibility"].GetString();
    attributes = json["attributes"].GetStringArray();
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json Variable::To() const
{
    Json json = Json::CreateObject();
    json.Add("name", name);
    json.Add("value", value);
    json.Add("type", type);
    json.Add("variablesReference", variablesReference);
    json.Add("presentationHint", presentationHint.To());
    return json;
}

void Variable::From(const Json& json)
{
    name = json["name"].GetString();
    value = json["value"].GetString();
    type = json["type"].GetString();
    variablesReference = json["variablesReference"].GetInteger();
    presentationHint.From(json["presentationHint"]);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json ScopesArguments::To() const
{
    Json json = Json::CreateObject();
    json.Add("frameId", frameId);
    return json;
}

void ScopesArguments::From(const Json& json) { frameId = json["frameId"].GetNumber(); }

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json ScopesRequest::To() const
{
    auto json = Request::To();
    json.Add("arguments", arguments.To());
    return json;
}

void ScopesRequest::From(const Json& json)
{
    Request::From(json);
    arguments.From(json["arguments"]);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json ScopesResponse::To() const
{
    auto json = Response::To();
    auto arr = json.AddObject("body").AddArray("scopes");
    for(const auto& scope : scopes) {
        arr.Add(scope.To());
    }
    return json;
}

void ScopesResponse::From(const Json& json)
{
    Response::From(json);
    auto arr = json["body"]["scopes"];
    size_t count = arr.GetCount();
    scopes.reserve(count);
    for(size_t i = 0; i < count; ++i) {
        Scope s;
        s.From(arr[i]);
        scopes.push_back(s);
    }
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json Scope::To() const
{
    auto json = Json::CreateObject();
    json.Add("name", name);
    json.Add("variablesReference", variablesReference);
    json.Add("expensive", expensive);
    return json;
}

void Scope::From(const Json& json)
{
    name = json["name"].GetString();
    variablesReference = json["variablesReference"].GetInteger();
    expensive = json["expensive"].GetBool();
}
// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json StackTraceArguments::To() const
{
    auto json = Json::CreateObject();
    json.Add("threadId", threadId);
#if 0
    json.Add("startFrame", startFrame);
    json.Add("levels", levels);
#endif
    return json;
}

void StackTraceArguments::From(const Json& json)
{
    threadId = json["threadId"].GetInteger();
#if 0
    startFrame = json["startFrame"].GetInteger();
    levels = json["levels"].GetInteger();
#endif
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json ValueFormat::To() const
{
    auto json = Json::CreateObject();
    json.Add("hex", hex);
    return json;
}

void ValueFormat::From(const Json& json) { hex = json["hex"].GetBool(); }

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json VariablesArguments::To() const
{
    auto json = Json::CreateObject();
    json.Add("variablesReference", (int)variablesReference);
    json.Add("count", count);
    json.Add("format", format.To());
    return json;
}

void VariablesArguments::From(const Json& json)
{
    variablesReference = json["variablesReference"].GetInteger();
    count = json["count"].GetInteger(0);
    format.From(json["format"]);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json StackTraceRequest::To() const
{
    auto json = Request::To();
    json.Add("arguments", arguments.To());
    return json;
}

void StackTraceRequest::From(const Json& json)
{
    Request::From(json);
    arguments.From(json["arguments"]);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json StackTraceResponse::To() const
{
    auto json = Response::To();
    auto arr = json.AddObject("body").AddArray("stackFrames");
    for(const auto& sf : stackFrames) {
        arr.Add(sf.To());
    }
    return json;
}

void StackTraceResponse::From(const Json& json)
{
    Response::From(json);
    auto arr = json["body"]["stackFrames"];
    size_t count = arr.GetCount();
    stackFrames.clear();
    stackFrames.reserve(count);
    for(size_t i = 0; i < count; ++i) {
        StackFrame sf;
        sf.From(arr[i]);
        stackFrames.push_back(sf);
    }
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json VariablesRequest::To() const
{
    auto json = Request::To();
    json.Add("arguments", arguments.To());
    return json;
}

void VariablesRequest::From(const Json& json)
{
    Request::From(json);
    arguments.From(json["arguments"]);
}

// ----------------------------------------
// ----------------------------------------
// ----------------------------------------

Json VariablesResponse::To() const
{
    auto json = Response::To();
    auto arr = json.AddObject("body").AddArray("variables");
    for(const auto& v : variables) {
        arr.Add(v.To());
    }
    return json;
}

void VariablesResponse::From(const Json& json)
{
    Response::From(json);
    auto arr = json["body"]["variables"];
    size_t count = arr.GetCount();
    variables.reserve(count);
    for(size_t i = 0; i < count; ++i) {
        Variable v;
        v.From(arr[i]);
        variables.push_back(v);
    }
}

void PauseArguments::From(const Json& json) { threadId = json["threadId"].GetInteger(threadId); }
Json PauseArguments::To() const
{
    auto json = Json::CreateObject();
    json.Add("threadId", threadId);
    return json;
}

Json PauseRequest::To() const
{
    auto json = Request::To();
    json.Add("arguments", arguments.To());
    return json;
}

void PauseRequest::From(const Json& json)
{
    Request::From(json);
    arguments.From(json["arguments"]);
}

void RunInTerminalRequestArguments::From(const Json& json)
{
    kind = json["kind"].GetString(kind);
    title = json["title"].GetString(title);
    args = json["args"].GetStringArray();
}

Json RunInTerminalRequestArguments::To() const
{
    auto json = Json::CreateObject();
    json.Add("kind", kind);
    json.Add("title", title);
    json.Add("args", args);
    return json;
}
Json RunInTerminalRequest::To() const
{
    auto json = Request::To();
    json.Add("arguments", arguments.To());
    return json;
}

void RunInTerminalRequest::From(const Json& json)
{
    Request::From(json);
    arguments.From(json["arguments"]);
}

Json RunInTerminalResponse::To() const
{
    RESPONSE_TO();
    ADD_BODY();
    ADD_BODY_PROP(processId);
    return json;
}

void RunInTerminalResponse::From(const Json& json)
{
    RESPONSE_FROM();
    READ_BODY();
    GET_BODY_PROP(processId, Number);
}

///
/// source request + args
///
void SourceArguments::From(const Json& json)
{
    source.From(json["source"]);
    sourceReference = json["sourceReference"].GetInteger(0);
}

Json SourceArguments::To() const
{
    Json json = Json::CreateObject();
    json.Add("source", source.To());
    if(sourceReference > 0) {
        json.Add("sourceReference", sourceReference);
    }
    return json;
}

void SourceRequest::From(const Json& json)
{
    REQUEST_FROM();
    READ_OBJ(arguments);
}

Json SourceRequest::To() const
{
    REQUEST_TO();
    ADD_OBJ(arguments);
    return json;
}

Json SourceResponse::To() const
{
    RESPONSE_TO();
    ADD_BODY();
    ADD_BODY_PROP(content);
    ADD_BODY_PROP(mimeType);
    return json;
}

void SourceResponse::From(const Json& json)
{
    RESPONSE_FROM();
    READ_BODY();
    GET_BODY_PROP(content, String);
    GET_BODY_PROP(mimeType, String);
}

Json EvaluateArguments::To() const
{
    Json json = Json::CreateObject();
    json.Add("expression", expression);
    if(frameId > 0) {
        json.Add("frameId", frameId);
    }
    json.Add("context", context);
    json.Add("format", format.To());
    return json;
}

void EvaluateArguments::From(const Json& json)
{
    expression = json["expression"].GetString(expression);
    frameId = json["frameId"].GetInteger(wxNOT_FOUND);
    context = json["context"].GetString(context);
    format.From(json["format"]);
}

Json EvaluateRequest::To() const
{
    REQUEST_TO();
    ADD_OBJ(arguments);
    return json;
}

void EvaluateRequest::From(const Json& json)
{
    REQUEST_FROM();
    READ_OBJ(arguments);
}

Json EvaluateResponse::To() const
{
    RESPONSE_TO();
    ADD_BODY();
    ADD_BODY_PROP(result);
    ADD_BODY_PROP(type);
    ADD_BODY_PROP(variablesReference);
    return json;
}

void EvaluateResponse::From(const Json& json)
{
    RESPONSE_FROM();
    READ_BODY();
    GET_BODY_PROP(result, String);
    GET_BODY_PROP(type, String);
    GET_BODY_PROP(variablesReference, Number);
}

void Module::From(const Json& json)
{
    // ID can be number or string
    int nId = json["id"].GetNumber(wxNOT_FOUND);
    if(nId == wxNOT_FOUND) {
        id = json["id"].GetString();
    } else {
        id << nId;
    }
    GET_PROP(name, String);
    GET_PROP(path, String);
    GET_PROP(version, String);
    GET_PROP(symbolStatus, String);
    GET_PROP(symbolFilePath, String);
    GET_PROP(dateTimeStamp, String);
    GET_PROP(addressRange, String);
    GET_PROP(isOptimized, Bool);
    GET_PROP(isUserCode, Bool);
}

Json Module::To() const
{
    CREATE_JSON();
    ADD_PROP(id);
    ADD_PROP(name);
    ADD_PROP(path);
    ADD_PROP(version);
    ADD_PROP(symbolStatus);
    ADD_PROP(symbolFilePath);
    ADD_PROP(dateTimeStamp);
    ADD_PROP(addressRange);
    ADD_PROP(isOptimized);
    ADD_PROP(isUserCode);
    return json;
}

Json ModuleEvent::To() const
{
    Json json = Event::To();
    Json body = json.AddObject("body");
    body.Add("reason", reason);
    body.AddObject("module", module.To());
    return json;
}

void ModuleEvent::From(const Json& json)
{
    Event::From(json);
    Json body = json["body"];
    reason = body["reason"].GetString();
    module.From(body["module"]);
}
}; // namespace dap
