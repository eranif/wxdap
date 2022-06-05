#ifndef CLCONNECTIONSTRING_H
#define CLCONNECTIONSTRING_H

#include <string>

namespace dap
{
class ConnectionString
{
public:
    enum eProtocol {
        kTcp,
        kUnixLocalSocket,
    };

protected:
    eProtocol m_protocol;
    std::string m_host;
    long m_port;
    std::string m_path;
    bool m_isOK;

protected:
    void DoParse(const std::string& connectionString);

public:
    ConnectionString(const std::string& connectionString);
    ~ConnectionString();

    void SetHost(const std::string& host) { this->m_host = host; }
    void SetIsOK(bool isOK) { this->m_isOK = isOK; }
    void SetPath(const std::string& path) { this->m_path = path; }
    void SetPort(long port) { this->m_port = port; }
    void SetProtocol(const eProtocol& protocol) { this->m_protocol = protocol; }
    const std::string& GetHost() const { return m_host; }
    bool IsOK() const { return m_isOK; }
    const std::string& GetPath() const { return m_path; }
    long GetPort() const { return m_port; }
    const eProtocol& GetProtocol() const { return m_protocol; }
};
};     // namespace dap
#endif // CLCONNECTIONSTRING_H
