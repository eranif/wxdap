#include "CommandLineParser.hpp"
#include "Driver.hpp"
#include "Log.hpp"
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

    // Open the log file
    Log::OpenStdout(Log::Developer);
    LOG_INFO() << "Started";

    try {
        // Initialize the dap library
        dap::Initialize();
        LOG_INFO() << "Listening on " << parser.GetConnectionString();

        dap::SocketServer server;
        server.Start(parser.GetConnectionString());
        LOG_INFO() << "Waiting for a new connection";

        dap::SocketBase::Ptr_t client = server.WaitForNewConnection();
        LOG_INFO() << "Connection established successfully";

        // Construct a GDB Driver
        Driver driver(parser);

        // The main loop:
        // - Check for any input from GDB and send it over JSONRpc to the client
        // - Check for any request from the client and pass it to the gdb
        dap::JsonRPC rpc;
        string network_buffer;
        bool initializeCompleted = false;
        while(driver.IsAlive()) {
            dap::ProtocolMessage::Ptr_t message = driver.Check();
            if(message) {
                // send it to the driver
                client->Send(message->To().Format());
            }

            // Attempt to read something from the network
            network_buffer.clear();
            if(client->Read(network_buffer, 10) == dap::SocketBase::kSuccess) {

                LOG_DEBUG1() << "Read: " << network_buffer;

                // Append the buffer to what we already have
                rpc.AppendBuffer(network_buffer);

                // Try to construct a message and process it
                dap::ProtocolMessage::Ptr_t request = rpc.ProcessBuffer();
                if(request) {
                    if(!initializeCompleted) {
                        dap::InitializeResponse response;
                        rpc.Send(response, client);
                        LOG_DEBUG() << "Initialization completed";
                    } else {
                    }
                }
            }
        }
    } catch(dap::SocketException& e) {
        LOG_ERROR() << "ERROR: " << e.what();
        return 1;
    }
    // We are done
    return 0;
}
