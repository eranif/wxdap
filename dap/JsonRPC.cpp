#include "JsonRPC.hpp"
#include "Exception.hpp"
#include "SocketServer.hpp"
#include "StringUtils.hpp"
#include <iostream>

dap::JsonRPC::JsonRPC() {}

dap::JsonRPC::~JsonRPC() {}

JSON dap::JsonRPC::DoProcessBuffer()
{
    if(m_buffer.empty()) {
        return JSON();
    }

    // Find the "Content-Length:" string
    unordered_map<wxString, wxString> headers;
    int headerSize = ReadHeaders(headers);
    if(headerSize == -1) {
        return JSON();
    }
    // We got the headers, check to see that we have the "Content-Length" one
    auto iter = headers.find("Content-Length");
    if(iter == headers.end()) {
        // this is a problem in the protocol. If we restore this section back to the buffer
        // we will simply stuck with it again later. So we remove it and return null
        m_buffer.erase(headerSize);
        cerr << "ERROR: Read complete header section. But no Content-Length header was found" << endl;
        return JSON();
    }

    wxString contentLength = iter->second;
    long msglen = std::atol(contentLength.c_str());
    if(msglen <= 0) {
        cerr << "ERROR: Invalid Content-Length header value: 0 or lower than 0" << endl;
        return JSON();
    }

    long buflen = m_buffer.length();
    if((headerSize + msglen) > buflen) {
        // not enough buffer
        return JSON();
    }

    // Read the payload into a separate buffer and remove the full message
    // from the m_buffer member
    wxString payload(m_buffer.begin() + headerSize, m_buffer.begin() + headerSize + msglen);
    m_buffer.erase(0, headerSize + msglen);
    return JSON::Parse(payload);
}

void dap::JsonRPC::ProcessBuffer(std::function<void(const JSON&, wxObject*)> callback, wxObject* o)
{
    JSON json = DoProcessBuffer();
    while(json.IsOK()) {
        if(json.IsOK()) {
            callback(json, o);
        }
        json = DoProcessBuffer();
    }
}

int dap::JsonRPC::ReadHeaders(unordered_map<wxString, wxString>& headers)
{
    size_t where = m_buffer.find("\r\n\r\n");
    if(where == wxString::npos) {
        return -1;
    }
    wxString headerSection = m_buffer.substr(0, where); // excluding the "\r\n\r\n"
    std::vector<wxString> lines = StringUtils::Split(headerSection, '\n');
    for(wxString& header : lines) {
        StringUtils::Trim(header);
        wxString name = StringUtils::BeforeFirst(header, ':');
        wxString value = StringUtils::AfterFirst(header, ':');
        headers.insert({ StringUtils::Trim(name), StringUtils::Trim(value) });
    }
    // return the headers section + the separator
    return (where + 4);
}

void dap::JsonRPC::SetBuffer(const wxString& buffer) { m_buffer = buffer; }

void dap::JsonRPC::AppendBuffer(const wxString& buffer) { m_buffer.append(buffer); }

void dap::JsonRPC::Send(ProtocolMessage& msg, Socket::Ptr_t conn) const
{
    if(!conn) {
        throw Exception("Invalid connection");
    }
    wxString network_buffer;
    wxString payload = msg.ToString();
    network_buffer = "Content-Length: ";
    network_buffer += to_string(payload.length());
    network_buffer += "\r\n\r\n";
    network_buffer += payload;
    conn->Send(network_buffer);
}

void dap::JsonRPC::Send(ProtocolMessage::Ptr_t msg, Socket::Ptr_t conn) const
{
    if(!msg) {
        throw Exception("Unable to send empty message");
    }
    if(!conn) {
        throw Exception("Invalid connection");
    }
    Send(*msg.get(), conn);
}
