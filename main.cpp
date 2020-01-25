#include "CommandLineParser.hpp"
#include "Driver.hpp"
#include "dap/JsonRPC.hpp"
#include "dap/Process.hpp"
#include "dap/SocketBase.hpp"
#include "dap/SocketServer.hpp"
#include "dap/StringUtils.hpp"
#include "dap/dap.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace std;

int main(int argc, char** argv)
{
    // Parse the command line arguments (exit if needed)
    CommandLineParser parser;
    parser.Parse(argc, argv);

    try {
        // Initialize the dap library
        dap::Initialize();

        dap::JsonRPC rpc;
        cout << "Waiting for connection on " << parser.GetConnectionString() << endl;
        rpc.ServerStart(parser.GetConnectionString());

        // Wait for new connection to arrive
        while(!rpc.WaitForNewConnection()) {
        }

        cout << "Connection established successfully" << endl;

        // Construct a GDB Driver
        Driver driver(parser);

        // The main loop:
        // - Check for any input from GDB and send it over JSONRpc to the client
        // - Check for any request from the client and pass it to the gdb
        while(driver.IsAlive()) {
            dap::ProtocolMessage::Ptr_t message = driver.Check();
            if(message) {
                // send it to the driver
                rpc.WriteMessge(message);
            }
            if(rpc.Read()) {
                dap::ProtocolMessage::Ptr_t request = rpc.ProcessBuffer();
                if(request) {
                    // Pass the request to the driver
                }
            }
        }
    } catch(dap::SocketException& e) {
        cerr << "ERROR: " << e.what() << endl;
        return 1;
    }
    // We are done
    return 0;
}
