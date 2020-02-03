#include "Client.hpp"
#include "SocketClient.hpp"
#include "dap.hpp"
#include <thread>

dap::Client::Client()
{
    Initialize();
    m_socket.reset(new SocketClient());
}

dap::Client::~Client() {}

void dap::Client::Connect(int timeoutSeconds)
{
    long ms = timeoutSeconds * 1000;
    while(ms) {
        if(!m_socket->As<dap::SocketClient>()->Connect("tcp://127.0.0.1:12345")) {
            this_thread::sleep_for(chrono::milliseconds(100));
            ms -= 100;
        }
    }
}
