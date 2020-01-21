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
    } catch(dap::SocketException& e) {
        cerr << e.what() << endl;
        return -1;
    }
    // We are done
    return 0;
}
