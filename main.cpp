#include "Driver.hpp"
#include "dap/Process.hpp"
#include "dap/SocketBase.hpp"
#include "dap/SocketServer.hpp"
#include "dap/StringUtils.hpp"
#include "dap/dap.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace std;
#define CHECK_STRING(str, expected_str)                               \
    if(str == expected_str) {                                         \
        cout << "Success: " << str << " == " << expected_str << endl; \
    } else {                                                          \
        cerr << "Error: " << str << " == " << expected_str << endl;   \
    }

#define CHECK_PTR(p, msg)                          \
    if(!p) {                                       \
        cerr << "Error: nullptr: " << msg << endl; \
        return -1;                                 \
    }

int main(int argc, char** argv)
{
    // Open stdin and wait for incoming for the initialize request
    dap::Initialize();

    try {
        // dap::SetBreakpointsRequest setBrkReq;
        // dap::SetBreakpointsResponse setBrkRes;
        // dap::BreakpointLocationsRequest brkLocReq;
        // dap::BreakpointLocationsResponse brkLocRes;
        // cout << setBrkReq.To().Format() << endl;
        // cout << setBrkRes.To().Format() << endl;
        // cout << brkLocReq.To().Format() << endl;
        // cout << brkLocRes.To().Format() << endl;
        //
        // dap::ProtocolMessage::Ptr_t obj;
        // obj = dap::ObjGenerator::Get().New("request", "cancel");
        // CHECK_PTR(obj, "cancel");
        //
        // obj = dap::ObjGenerator::Get().New("request", "initialize");
        // CHECK_PTR(obj, "initialize");
        //
        // obj = dap::ObjGenerator::Get().New("request", "configurationDone");
        // CHECK_PTR(obj, "configurationDone");
        //
        // obj = dap::ObjGenerator::Get().New("request", "launch");
        // CHECK_PTR(obj, "launch");
        //
        // obj = dap::ObjGenerator::Get().New("request", "disconnect");
        // CHECK_PTR(obj, "disconnect");
        //
        // obj = dap::ObjGenerator::Get().New("request", "breakpointLocations");
        // CHECK_PTR(obj, "breakpointLocations");
        //
        // obj = dap::ObjGenerator::Get().New("request", "setBreakpoints");
        // CHECK_PTR(obj, "setBreakpoints");
        //
        // obj = dap::ObjGenerator::Get().New("request", "continue");
        // CHECK_PTR(obj, "continue");
        //
        // obj = dap::ObjGenerator::Get().New("event", "stopped");
        // CHECK_PTR(obj, "stopped");

    } catch(dap::SocketException& e) {
        cerr << e.what() << endl;
        return -1;
    }
    // We are done
    return 0;
}
