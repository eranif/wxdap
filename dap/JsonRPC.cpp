#include "JsonRPC.hpp"
#include "SocketServer.hpp"
#include "StringUtils.hpp"
#include <iostream>

dap::JsonRPC::JsonRPC() {}

dap::JsonRPC::~JsonRPC() {}

void dap::JsonRPC::ServerStart(const string& connectString)
{
    m_acceptSocket.reset(new dap::SocketServer());
    m_acceptSocket->SetCloseOnExit(true);
    m_acceptSocket->As<dap::SocketServer>()->Start(connectString);
}

bool dap::JsonRPC::WaitForNewConnection()
{
    // wait for new connection for 1 second
    m_client = m_acceptSocket->As<dap::SocketServer>()->WaitForNewConnection(1);
    return m_client != nullptr;
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
    return nullptr;
}

void dap::JsonRPC::WriteMessge(ProtocolMessage::Ptr_t message) {}

bool dap::JsonRPC::Read()
{
    if(!m_client) {
        throw SocketException("Connection is not opened");
    }
    string buffer;
    if(m_client->Read(buffer, 1) == SocketBase::kSuccess) {
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
