#ifndef COMMANDLINEPARSER_HPP
#define COMMANDLINEPARSER_HPP

#include <getopt.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class CommandLineParser
{
    std::vector<std::string> m_arguments;
    bool m_verbose = false;
    std::string m_host;
    int m_port = 35670;
    std::string m_debuggerExec;

public:
    CommandLineParser();
    virtual ~CommandLineParser();

    void Parse(int argc, char** argv);
    void PrintUsage(const char* exename, struct option long_options[]);

    void SetHost(const std::string& host) { this->m_host = host; }
    void SetPort(int port) { this->m_port = port; }
    void SetVerbose(bool verbose) { this->m_verbose = verbose; }
    const std::string& GetHost() const { return m_host; }
    int GetPort() const { return m_port; }
    bool IsVerbose() const { return m_verbose; }
    std::string GetConnectionString() const;

    void SetDebuggerExec(const std::string& debuggerExec) { this->m_debuggerExec = debuggerExec; }
    const std::string& GetDebuggerExec() const { return m_debuggerExec; }
};
#endif // COMMANDLINEPARSER_HPP
