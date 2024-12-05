#include "JsonRPC.hpp"

#include "Exception.hpp"
#include "Log.hpp"
#include "SocketServer.hpp"
#include "StringUtils.hpp"

#include <iostream>

dap::JsonRPC::JsonRPC() {}

dap::JsonRPC::~JsonRPC() {}

dap::Json dap::JsonRPC::DoProcessBuffer()
{
    if (m_buffer.empty()) {
        return {};
    }

    // Find the "Content-Length:" string
    std::unordered_map<std::string, std::string> headers;
    int headerSize = ReadHeaders(headers);
    if (headerSize == -1) {
        return {};
    }

    LOG_DEBUG() << "Headers:" << headers.size() << endl;
    for (const auto& [k, v] : headers) {
        LOG_DEBUG() << k << ":" << v << endl;
    }

    // We got the headers, check to see that we have the "Content-Length" one
    auto iter = headers.find("Content-Length");
    if (iter == headers.end()) {
        // this is a problem in the protocol. If we restore this section back to the buffer
        // we will simply stuck with it again later. So we remove it and return null
        m_buffer.erase(headerSize);
        LOG_ERROR() << "ERROR: Read complete header section. But no Content-Length header was found" << endl;
        return {};
    }

    std::string contentLength = iter->second;
    long msglen = std::atol(contentLength.c_str());
    if (msglen <= 0) {
        LOG_ERROR() << "ERROR: Invalid Content-Length header value: 0 or lower than 0" << endl;
        return {};
    }

    long buflen = m_buffer.length();
    if ((headerSize + msglen) > buflen) {
        LOG_INFO() << "Not enough buffer" << endl;
        // not enough buffer
        return {};
    }

    // Read the payload into a separate buffer and remove the full message
    // from the m_buffer member
    std::string payload(m_buffer.begin() + headerSize, m_buffer.begin() + headerSize + msglen);
    m_buffer.erase(0, headerSize + msglen);
    return Json::Parse(payload);
}

void dap::JsonRPC::ProcessBuffer(std::function<void(const Json&, wxObject*)> callback, wxObject* o)
{
    Json json = DoProcessBuffer();
    while (json.IsOK()) {
        if (json.IsOK()) {
            callback(json, o);
        }
        json = DoProcessBuffer();
    }
}

int dap::JsonRPC::ReadHeaders(unordered_map<std::string, std::string>& headers)
{
    size_t where = m_buffer.find("\r\n\r\n");
    if (where == wxString::npos) {
        return -1;
    }
    std::string headerSection = m_buffer.substr(0, where); // excluding the "\r\n\r\n"
    std::vector<std::string> lines = DapStringUtils::Split(headerSection, "\n");
    for (std::string& header : lines) {
        DapStringUtils::Trim(header);
        std::string name = DapStringUtils::BeforeFirst(header, ':');
        std::string value = DapStringUtils::AfterFirst(header, ':');
        headers.insert({ DapStringUtils::Trim(name), DapStringUtils::Trim(value) });
    }
    // return the headers section + the separator
    return (where + 4);
}

void dap::JsonRPC::SetBuffer(const std::string& buffer) { m_buffer = buffer; }

void dap::JsonRPC::AppendBuffer(const std::string& buffer) { m_buffer.append(buffer); }
