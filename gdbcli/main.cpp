#include "dap/Exception.hpp"
#include "dap/JsonRPC.hpp"
#include "dap/SocketClient.hpp"
#include <iostream>
#include <stdio.h>
#include <string>
#include <thread>

using namespace std;
int main(int argc, char** argv)
{
    try {
        cout << "Starting client..." << endl;
        dap::Initialize();
        dap::SocketBase::Ptr_t cli(new dap::SocketClient());

        // Wait until we connect
        while(!cli->As<dap::SocketClient>()->Connect("tcp://127.0.0.1:12345"))
            ;
        cout << "Connection established successfully" << endl;

        // Send initialize message
        dap::InitializeRequest req;
        dap::JsonRPC rpc;
        req.arguments.clientID = "gdbcli";
        rpc.Send(req, cli);

        // Use a flag to indicate if we lost connection to the server
        atomic_bool terminated;
        terminated.store(false);

        // Wait for the response
        dap::Queue<string> inputQueue;
        atomic_bool shutdown;

        shutdown.store(false);

        // Start a reader thread that will continously read from the socket
        // and put the input buffer in queue
        thread readerThread([&]() {
            while(!shutdown.load()) {
                try {
                    string content;
                    if(cli->Read(content)) {
                        inputQueue.push(content);
                    }
                } catch(dap::Exception& e) {
                    cerr << "Connection error: " << e.What() << endl;
                    terminated.store(true);
                    break;
                }
            }
        });

        // We are expecting 'initialized' request
        enum eState { kWaitingInitResponse, kWaitingInitEvent, kExecute };
        eState state = kWaitingInitResponse;

        // This part is for performing the initialization part of the debugger
        while(!terminated.load() && (state != kExecute)) {
            string buffer = inputQueue.pop(chrono::milliseconds(1));
            if(!buffer.empty()) {
                // got something on the network
                rpc.AppendBuffer(buffer);
                dap::ProtocolMessage::Ptr_t msg = rpc.ProcessBuffer();
                if(msg) {
                    switch(state) {
                    case kWaitingInitResponse:
                        if(msg->type == "response" && msg->As<dap::Response>()->command == "initialize") {
                            cout << "Received initialized response" << endl;
                            state = kWaitingInitEvent;
                        }
                        break;
                    case kWaitingInitEvent:
                        if(msg->type == "event" && msg->As<dap::Event>()->event == "initialized") {
                            cout << "Received initialized event" << endl;
                            state = kExecute;
                        }
                        break;
                    case kExecute:
                        // handle anything else here
                        break;
                    }
                }
            }
        }

        /// Prepare list of commands
        vector<dap::ProtocolMessage::Ptr_t> commands;

        // We start with launch request which contains the executable to debug
        dap::LaunchRequest* launchRequest = new dap::LaunchRequest();
        launchRequest->seq = 0; // command sequence
        launchRequest->arguments.debuggee = { "C:\\Users\\Eran\\Documents\\AmitTest\\build-Debug\\bin\\AmitTest.exe" };
        commands.push_back(dap::ProtocolMessage::Ptr_t(launchRequest));

        // Add set breakpoint
        dap::SetBreakpointsRequest* setBreakpoints = new dap::SetBreakpointsRequest();
        // Set breakpoint on main.cpp:10 and on main.cpp:12
        setBreakpoints->seq = 1; // command sequence
        dap::SourceBreakpoint source;
        source.line = 10;
        setBreakpoints->arguments.breakpoints.push_back(source);
        source.line = 12;
        setBreakpoints->arguments.breakpoints.push_back(source);

        setBreakpoints->arguments.source.path = "main.cpp";
        commands.push_back(dap::ProtocolMessage::Ptr_t(setBreakpoints));

        //-----------------------------------------------------
        // The main loop
        //-----------------------------------------------------
        // Now that the initialization is completed, run the main loop
        while(!terminated.load() && !commands.empty()) {
            // Wait for user input
            dap::ProtocolMessage::Ptr_t req = commands.front();
            commands.erase(commands.begin());

            // Send the request
            rpc.Send(req, cli);
            cout << "==> " << req->To().Format() << endl;
            string buffer = inputQueue.pop(chrono::milliseconds(1));
            if(!buffer.empty()) {
                // got something on the network
                rpc.AppendBuffer(buffer);
                dap::ProtocolMessage::Ptr_t msg = rpc.ProcessBuffer();
                if(msg) {
                    cout << "<== " << msg->To().Format() << endl;
                }
            }
        }
        readerThread.join();

    } catch(dap::Exception& e) {
        cerr << "Error: " << e.What() << endl;
    }
    return 0;
}
