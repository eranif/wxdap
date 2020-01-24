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

using namespace std;
int main(int argc, char** argv)
{
    // Initialize the dap library
    dap::Initialize();
    dap::Process* gdb = dap::ExecuteProcess("C:\\compilers\\mingw64\\bin\\gdb.exe");
    size_t counter = 0;
    while(gdb->IsAlive() && (counter < 10)) {
        string o, e;
        if(gdb->Read(o, e)) {
            if(!o.empty()) {
                cout << o << endl;
            }
            if(!e.empty()) {
                cerr << e << endl;
            }
        }
        this_thread::sleep_for(chrono::milliseconds(10));
        gdb->Write("help");
        ++counter;
    }

    delete gdb;
    // We are done
    return 0;
}
