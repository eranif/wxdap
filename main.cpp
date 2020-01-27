#include "CommandLineParser.hpp"
#include "Driver.hpp"
#include "dap/JsonRPC.hpp"
#include "dap/Log.hpp"
#include "dap/Process.hpp"
#include "dap/ServerProtocol.hpp"
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
    dap::Log::OpenStdout(dap::Log::Developer);
    LOG_INFO() << "Started";

    try {
        // Initialize the dap library
        dap::Initialize();
        LOG_INFO() << "Listening on " << parser.GetConnectionString();

        dap::SocketServer socketServer;
        socketServer.Start(parser.GetConnectionString());
        LOG_INFO() << "Waiting for a new connection";

        dap::SocketBase::Ptr_t client = socketServer.WaitForNewConnection();
        LOG_INFO() << "Connection established successfully";

        // Construct a GDB Driver
        Driver driver(parser);

        dap::ServerProtocol server;
        server.Initialize(client);

        // The main loop:
        // - Check for any input from GDB and send it over JSONRpc to the client
        // - Check for any request from the client and pass it to the gdb
        while(driver.IsAlive()) {
            dap::ProtocolMessage::Ptr_t message = driver.Check();
            if(message) {
                // send it to the driver
                client->Send(message->To().Format());
            }
        }
    } catch(dap::SocketException& e) {
        LOG_ERROR() << "ERROR: " << e.what();
        return 1;
    }
    // We are done
    return 0;
}
