#include "dap/JsonRPC.hpp"
#include "dap/dap.hpp"
#include "tester.h"
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <string>
#include "dap/StringUtils.hpp"

using namespace std;

#define CHECK_REQUEST(obj, str) \
    CHECK_CONDITION(obj, str);  \
    CHECK_STRING(obj->As<dap::Request>()->command.c_str().AsChar(), str)

#define CHECK_RESPONSE(obj, str) \
    CHECK_CONDITION(obj, str);   \
    CHECK_STRING(obj->As<dap::Response>()->command.c_str().AsChar(), str)

#define CHECK_EVENT(obj, str)  \
    CHECK_CONDITION(obj, str); \
    CHECK_STRING(obj->As<dap::Event>()->event.c_str().AsChar(), str)

int main(int, char**)
{
    dap::Initialize();

    Tester::Instance()->RunTests();
    Tester::Release();
    return 0;
}

TEST_FUNC(Check_Request_Allocations)
{
    dap::ProtocolMessage::Ptr_t obj;

    // Requests
    obj = dap::ObjGenerator::Get().New("request", "cancel");
    CHECK_REQUEST(obj, "cancel");
    obj = dap::ObjGenerator::Get().New("request", "initialize");
    CHECK_REQUEST(obj, "initialize");
    obj = dap::ObjGenerator::Get().New("request", "configurationDone");
    CHECK_REQUEST(obj, "configurationDone");
    obj = dap::ObjGenerator::Get().New("request", "launch");
    CHECK_REQUEST(obj, "launch");
    obj = dap::ObjGenerator::Get().New("request", "disconnect");
    CHECK_REQUEST(obj, "disconnect");
    obj = dap::ObjGenerator::Get().New("request", "breakpointLocations");
    CHECK_REQUEST(obj, "breakpointLocations");
    obj = dap::ObjGenerator::Get().New("request", "setBreakpoints");
    CHECK_REQUEST(obj, "setBreakpoints");
    obj = dap::ObjGenerator::Get().New("request", "continue");
    CHECK_REQUEST(obj, "continue");
    return true;
}

TEST_FUNC(Check_Response_Allocations)
{
    dap::ProtocolMessage::Ptr_t obj;
    // Responses
    obj = dap::ObjGenerator::Get().New("response", "initialize");
    CHECK_RESPONSE(obj, "initialize");
    obj = dap::ObjGenerator::Get().New("response", "cancel");
    CHECK_RESPONSE(obj, "cancel");
    obj = dap::ObjGenerator::Get().New("response", "configurationDone");
    CHECK_RESPONSE(obj, "configurationDone");
    obj = dap::ObjGenerator::Get().New("response", "launch");
    CHECK_RESPONSE(obj, "launch");
    obj = dap::ObjGenerator::Get().New("response", "disconnect");
    CHECK_RESPONSE(obj, "disconnect");
    obj = dap::ObjGenerator::Get().New("response", "breakpointLocations");
    CHECK_RESPONSE(obj, "breakpointLocations");
    obj = dap::ObjGenerator::Get().New("response", "continue");
    CHECK_RESPONSE(obj, "continue");
    obj = dap::ObjGenerator::Get().New("response", "setBreakpoints");
    CHECK_RESPONSE(obj, "setBreakpoints");
    return true;
}

TEST_FUNC(Check_Event_Allocations)
{
    dap::ProtocolMessage::Ptr_t obj;
    // Events
    obj = dap::ObjGenerator::Get().New("event", "initialized");
    CHECK_EVENT(obj, "initialized");
    obj = dap::ObjGenerator::Get().New("event", "stopped");
    CHECK_EVENT(obj, "stopped");
    obj = dap::ObjGenerator::Get().New("event", "continued");
    CHECK_EVENT(obj, "continued");
    obj = dap::ObjGenerator::Get().New("event", "exited");
    CHECK_EVENT(obj, "exited");
    obj = dap::ObjGenerator::Get().New("event", "output");
    CHECK_EVENT(obj, "output");
    obj = dap::ObjGenerator::Get().New("event", "process");
    CHECK_EVENT(obj, "process");
    obj = dap::ObjGenerator::Get().New("event", "stopped");
    CHECK_EVENT(obj, "stopped");
    obj = dap::ObjGenerator::Get().New("event", "terminated");
    CHECK_EVENT(obj, "terminated");
    obj = dap::ObjGenerator::Get().New("event", "thread");
    CHECK_EVENT(obj, "thread");
    return true;
}

TEST_FUNC(Check_Parsing_JSON_RPC_Message)
{
    const std::string jsonStr = "{\n"
                                "    \"seq\": 153,\n"
                                "    \"type\": \"request\",\n"
                                "    \"command\": \"next\",\n"
                                "    \"arguments\": {\n"
                                "        \"threadId\": 3\n"
                                "    }\n"
                                "}";
    const string header = "Content-Length: " + std::to_string(jsonStr.size()) +
                          "\r\n"
                          "\r\n";

    dap::JsonRPC rpc;
    rpc.SetBuffer(header + jsonStr);

    int count = 0;
    rpc.ProcessBuffer([&](const dap::Json& json, wxObject*) {

        CHECK_NUMBER(json["seq"].GetNumber(), 153);
        CHECK_STRING(json["type"].GetString().c_str().AsChar(), "request");
        CHECK_STRING(json["command"].GetString().c_str().AsChar(), "next");
        CHECK_NUMBER(json["arguments"]["threadId"].GetNumber(), 3);
        dap::NextRequest nextRequest;
        nextRequest.From(json);
        CHECK_NUMBER(nextRequest.seq, 153);
        CHECK_STRING(nextRequest.type.c_str().AsChar(), "request");
        CHECK_STRING(nextRequest.command.c_str().AsChar(), "next");
        CHECK_NUMBER(nextRequest.arguments.threadId, 3);

        ++count;
        return true;
    }, nullptr);
    CHECK_NUMBER(count, 1);
    return true;
}
