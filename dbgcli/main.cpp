#include "dap/Client.hpp"
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
        dap::Client client;
        if(!client.Connect(10)) {
            cerr << "Error: failed to connect to server";
            exit(1);
        }
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
                cout << "<== " << msg->To().Format() << endl;
                msg = client.Check();
            }
            this_thread::sleep_for(chrono::milliseconds(1));
        }

    } catch(dap::Exception& e) {
        cerr << "Error: " << e.What() << endl;
    }
    return 0;
}
