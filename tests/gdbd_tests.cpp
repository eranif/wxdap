#include "../dap/dap.hpp"
#include "tester.h"
#include <cstdio>
#include <cstdlib>
#include <string.h>

#define CHECK_REQUEST(obj, str) \
    CHECK_CONDITION(obj, str);  \
    CHECK_STRING(obj->As<dap::Request>()->command.c_str(), str)

#define CHECK_RESPONSE(obj, str) \
    CHECK_CONDITION(obj, str);   \
    CHECK_STRING(obj->As<dap::Response>()->command.c_str(), str)

#define CHECK_EVENT(obj, str)  \
    CHECK_CONDITION(obj, str); \
    CHECK_STRING(obj->As<dap::Event>()->event.c_str(), str)

int main(int argc, char** argv)
{
    dap::Initialize();

    Tester::Instance()->RunTests();
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
