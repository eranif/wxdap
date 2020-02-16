#include "Client.hpp"
#include "Exception.hpp"
#include "SocketClient.hpp"
#include "dap.hpp"
#include <iostream>
#include <thread>

dap::Client::Client()
{
    dap::Initialize();
    m_socket.reset(new SocketClient());
    m_shutdown.store(false);
    m_terminated.store(false);
}

dap::Client::~Client()
{
    if(m_readerThread) {
        m_shutdown.store(true);
        m_readerThread->join();
        delete m_readerThread;
    }
}

bool dap::Client::Connect(int timeoutSeconds)
{
    long loops = timeoutSeconds;
#ifndef _WIN32
    loops *= 1000;
#endif
    while(loops) {
        if(!m_socket->As<SocketClient>()->Connect("tcp://127.0.0.1:12345")) {
            this_thread::sleep_for(chrono::milliseconds(1));
            --loops;
        } else {
            return true;
        }
    }
    return false;
}

void dap::Client::Initialize()
{
    // Send initialize request
    InitializeRequest req;
    req.arguments.clientID = "dbgcli";
    m_rpc.Send(req, m_socket);

    // Start a reader thread
    // and put the input buffer in queue
    m_readerThread = new thread([this]() {
        while(!m_shutdown.load()) {
            try {
                string content;
                if(m_socket->Read(content)) {
                    m_inputQueue.push(content);
                }
            } catch(Exception& e) {
                cerr << "Connection error: " << e.What() << endl;
                m_terminated.store(true);
                break;
            }
        }
    });

    // We are expecting 'initialized' request
    enum eState { kWaitingInitResponse, kWaitingInitEvent, kExecute };
    eState state = kWaitingInitResponse;

    // This part is for performing the initialization part of the debugger
    while(!m_terminated.load() && (state != kExecute)) {
        string buffer = m_inputQueue.pop(chrono::milliseconds(1));
        if(!buffer.empty()) {
            // got something on the network
            m_rpc.AppendBuffer(buffer);
            // ProcessBuffer is called for all compeleted JSON messages found in 'buffer'
            m_rpc.ProcessBuffer([&](JSON json) {
                // construct a message from the JSON
                ProtocolMessage::Ptr_t msg = ObjGenerator::Get().FromJSON(json);
                if(msg) {
                    switch(state) {
                    case kWaitingInitResponse:
                        if(msg->type == "response" && msg->As<Response>()->command == "initialize") {
                            cout << "Received initialized response" << endl;
                            state = kWaitingInitEvent;
                        }
                        break;
                    case kWaitingInitEvent:
                        if(msg->type == "event" && msg->As<Event>()->event == "initialized") {
                            cout << "Received initialized event" << endl;
                            state = kExecute;
                        }
                        break;
                    case kExecute:
                        // handle anything else here
                        break;
                    }
                }
            });
        }
    }

    if(m_terminated.load()) {
        // connection lost to server
        throw Exception("Connection closed");
    }

    // Initialized completed successfully
    // we got both Initialized response & Initialized event
}

bool dap::Client::IsConnected() const { return m_readerThread && !m_terminated.load(); }

void dap::Client::SetBreakpointsFile(const string& file, const vector<dap::SourceBreakpoint>& lines)
{
    // Now that the initialize is done, we can call 'setBreakpoints' command
    SetBreakpointsRequest* setBreakpoints = new SetBreakpointsRequest();
    setBreakpoints->seq = GetNextSequence(); // command sequence
    setBreakpoints->arguments.breakpoints = lines;
    setBreakpoints->arguments.source.path = file;
    m_rpc.Send(ProtocolMessage::Ptr_t(setBreakpoints), m_socket);
}

void dap::Client::ConfigurationDone()
{
    ConfigurationDoneRequest* configDone = new ConfigurationDoneRequest();
    configDone->seq = GetNextSequence();
    m_rpc.Send(ProtocolMessage::Ptr_t(configDone), m_socket);
}

void dap::Client::Launch(const vector<string>& cmd)
{
    LaunchRequest* launchRequest = new LaunchRequest();
    launchRequest->seq = GetNextSequence(); // command sequence
    launchRequest->arguments.debuggee = cmd;
    m_rpc.Send(ProtocolMessage::Ptr_t(launchRequest), m_socket);
}

void dap::Client::Check(function<void(JSON)> callback)
{
    string buffer = m_inputQueue.pop(chrono::milliseconds(1));
    if(!buffer.empty()) {
        // got something on the network
        m_rpc.AppendBuffer(buffer);
        m_rpc.ProcessBuffer(callback);
    }
}

void dap::Client::GetThreads()
{
    ThreadsRequest* threadsRequest = new ThreadsRequest();
    threadsRequest->seq = GetNextSequence();
    m_rpc.Send(ProtocolMessage::Ptr_t(threadsRequest), m_socket);
}
