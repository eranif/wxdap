#include "JsonRPC.hpp"

#include "Exception.hpp"
#include "SocketServer.hpp"
#include "StringUtils.hpp"

#include <iostream>

dap::JsonRPC::JsonRPC() {}

dap::JsonRPC::~JsonRPC() {}

dap::Json dap::JsonRPC::DoProcessBuffer()
{
    if(m_buffer.empty()) {
        return Json();
    }

    // Find the "Content-Length:" string
    unordered_map<wxString, wxString> headers;
    int headerSize = ReadHeaders(headers);
    if(headerSize == -1) {
        return Json();
    }
    // We got the headers, check to see that we have the "Content-Length" one
    auto iter = headers.find("Content-Length");
    if(iter == headers.end()) {
        // this is a problem in the protocol. If we restore this section back to the buffer
        // we will simply stuck with it again later. So we remove it and return null
        m_buffer.erase(headerSize);
        cerr << "ERROR: Read complete header section. But no Content-Length header was found" << endl;
        return Json();
    }

    wxString contentLength = iter->second;
    long msglen = std::atol(contentLength.c_str());
    if(msglen <= 0) {
        cerr << "ERROR: Invalid Content-Length header value: 0 or lower than 0" << endl;
        return Json();
    }

    long buflen = m_buffer.length();
    if((headerSize + msglen) > buflen) {
        // not enough buffer
        return Json();
    }

    // Read the payload into a separate buffer and remove the full message
    // from the m_buffer member
    wxString payload(m_buffer.begin() + headerSize, m_buffer.begin() + headerSize + msglen);
    m_buffer.erase(0, headerSize + msglen);
    return Json::Parse(payload);
}

void dap::JsonRPC::ProcessBuffer(std::function<void(const Json&, wxObject*)> callback, wxObject* o)
{
    Json json = DoProcessBuffer();
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
    std::vector<wxString> lines = DapStringUtils::Split(headerSection, '\n');
    for(wxString& header : lines) {
        DapStringUtils::Trim(header);
        wxString name = DapStringUtils::BeforeFirst(header, ':');
        wxString value = DapStringUtils::AfterFirst(header, ':');
        headers.insert({ DapStringUtils::Trim(name), DapStringUtils::Trim(value) });
    }
    // return the headers section + the separator
    return (where + 4);
}

void dap::JsonRPC::SetBuffer(const wxString& buffer) { m_buffer = buffer; }

void dap::JsonRPC::AppendBuffer(const wxString& buffer) { m_buffer.append(buffer); }
