#include "dap/Client.hpp"
#include "dap/Exception.hpp"
#include "dap/JsonRPC.hpp"
#include "dap/Log.hpp"
#include "dap/SocketClient.hpp"
#include <iostream>
#include <stdio.h>
#include <string>
#include <thread>

using namespace std;
int main(int argc, char** argv)
{
    try {
        dap::Log::OpenStdout(dap::Log::Dbg);
        LOG_INFO() << "Starting client...";
        dap::Client client;
        if(!client.Connect(10)) {
            LOG_ERROR() << "Error: failed to connect to server";
            exit(1);
        }
        LOG_INFO() << "Connected!";
        
        client.Initialize();
        client.SetBreakpointsFile("main.cpp", { { 10, "" }, { 12, "" } });
        client.ConfigurationDone();
        client.Launch({ "C:\\Users\\Eran\\Documents\\AmitTest\\build-Debug\\bin\\AmitTest.exe" });

        //-----------------------------------------------------
        // The main loop
        //-----------------------------------------------------
        // Now that the initialization is completed, run the main loop
        while(client.IsConnected()) {
            auto msg = client.Check();
            while(msg) {
                LOG_DEBUG() << "<== " << msg->To().Format();
                // Stopped cause of breakpoint
                if(msg->AsEvent() && msg->AsEvent()->event == "stopped") {
                    LOG_INFO() << "Stopped." << msg->As<dap::StoppedEvent>()->text;
                }
                msg = client.Check();
            }
            this_thread::sleep_for(chrono::milliseconds(1));
        }

    } catch(dap::Exception& e) {
        LOG_ERROR() << "Error: " << e.What();
    }
    return 0;
}
