#include "JsonRPC.hpp"
#include "SocketServer.hpp"
#include "StringUtils.hpp"
#include <iostream>

void dap::Reader::Cleanup()
{
    shutdown_flag.store(true);
    if(thr) {
        thr->join();
        delete thr;
        thr = nullptr;
    }
    shutdown_flag.store(false);
    terminated_flag.store(false);
}

dap::JsonRPC::JsonRPC() {}

dap::JsonRPC::~JsonRPC() { m_reader.Cleanup(); }

void dap::JsonRPC::ServerStart(const string& connectString)
{
    m_acceptSocket.reset(new dap::SocketServer());
    m_acceptSocket->SetCloseOnExit(true);
    m_acceptSocket->As<dap::SocketServer>()->Start(connectString);
}

bool dap::JsonRPC::WaitForNewConnection()
{
    // wait for new connection for 1 second
    if(m_reader.thr) {
        return false;
    }
    m_reader.conn = m_acceptSocket->As<dap::SocketServer>()->WaitForNewConnectionRaw(1);
    if(m_reader.conn) {
        m_reader.shutdown_flag.store(false);
        m_reader.thr = new thread(
            [](Reader* reader, Queue<string>& Q) {
                try {
                    while(!reader->shutdown_flag.load()) {
                        string content;
                        if(reader->conn->Read(content, 1) == SocketBase::kSuccess) {
                            Q.push(content);
                        }
                    }
                    reader->Cleanup();

                } catch(SocketException& e) {
                    cerr << "Socket error: " << e.what() << endl;
                    reader->terminated_flag.store(true);
                }
            },
            &m_reader, ref(m_incQueue));
        return true;
    }
    return false;
}

dap::ProtocolMessage::Ptr_t dap::JsonRPC::ProcessBuffer()
{
    if(m_buffer.empty()) {
        return nullptr;
    }
    // Find the "Content-Length:" string
    unordered_map<string, string> headers;
    int headerSize = ReadHeaders(headers);
    if(headerSize == -1) {
        return nullptr;
    }
    // We got the headers, check to see that we have the "Content-Length" one
    auto iter = headers.find("Content-Length");
    if(iter == headers.end()) {
        // this is a problem in the protocol. If we restore this section back to the buffer
        // we will simply stuck with it again later. So we remove it and return null
        m_buffer.erase(headerSize);
        cerr << "ERROR: Read complete header section. But no Content-Length header was found" << endl;
        return nullptr;
    }

    string contentLength = iter->second;
    long msglen = std::atol(contentLength.c_str());
    if(msglen <= 0) {
        cerr << "ERROR: Invalid Content-Length header value: 0 or lower than 0" << endl;
        return nullptr;
    }

    long buflen = m_buffer.length();
    if((headerSize + msglen) > buflen) {
        // not enough buffer
        return nullptr;
    }

    // Read the payload into a separate buffer and remove the full message
    // from the m_buffer member
    string payload(m_buffer.begin() + headerSize, m_buffer.begin() + headerSize + msglen);
    m_buffer.erase(0, headerSize + msglen);

    JSON root(payload);
    if(!root.isOk()) {
        return nullptr;
    }
    JSONItem json = root.toElement();
    string type = json.property("type").toString();
    string command = (type == "event") ? json.property("event").toString() : json.property("command").toString();

    // Allocate
    ProtocolMessage::Ptr_t message = ObjGenerator::Get().New(type, command);
    if(!message) {
        return nullptr;
    }
    message->From(json);
    return message;
}

void dap::JsonRPC::WriteMessge(ProtocolMessage::Ptr_t message)
{
    if(!m_reader.conn) {
        throw SocketException("ERROR: WriteMessge() error: connection not opened");
    }
    if(!message) {
        throw SocketException("ERROR: WriteMessge() error: null message");
    }
    // Write the message over the socket
    m_reader.conn->Send(message->To().Format());
}

bool dap::JsonRPC::Read()
{
    if(m_reader.terminated_flag.load()) {
        m_reader.Cleanup();
        throw SocketException("ERROR: connection reset by peer");
    }

    string buffer = m_incQueue.pop(chrono::milliseconds(1));
    if(!buffer.empty()) {
        m_buffer.append(buffer);
    }
    return !buffer.empty();
}

int dap::JsonRPC::ReadHeaders(unordered_map<string, string>& headers)
{
    size_t where = m_buffer.find("\r\n\r\n");
    if(where == string::npos) {
        return -1;
    }
    string headerSection = m_buffer.substr(0, where); // excluding the "\r\n\r\n"
    vector<string> lines = StringUtils::Split(headerSection, '\n');
    for(string& header : lines) {
        StringUtils::Trim(header);
        string name = StringUtils::BeforeFirst(header, ':');
        string value = StringUtils::AfterFirst(header, ':');
        headers.insert({ StringUtils::Trim(name), StringUtils::Trim(value) });
    }
    // return the headers section + the separator
    return (where + 4);
}

void dap::JsonRPC::SetBuffer(const string& buffer) { m_buffer = buffer; }
