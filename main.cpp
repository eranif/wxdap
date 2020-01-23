#include "Driver.hpp"
#include "dap/Process.hpp"
#include "dap/SocketBase.hpp"
#include "dap/SocketServer.hpp"
#include "dap/StringUtils.hpp"
#include "dap/dap.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "dap/JsonRPC.hpp"

using namespace std;
int main(int argc, char** argv)
{
    // Initialize the dap library
    dap::Initialize();
    
    dap::JsonRPC* server = new dap::JsonRPC();
    server->ServerStart("tcp://127.0.0.1:12345");
    
    try {
    } catch(dap::SocketException& e) {
        cerr << e.what() << endl;
        return -1;
    }
    // We are done
    return 0;
}
