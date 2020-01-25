#ifndef COMMANDLINEPARSER_HPP
#define COMMANDLINEPARSER_HPP

#include <getopt.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;
class CommandLineParser
{
    vector<string> m_arguments;
    bool m_verbose = false;
    string m_host;
    int m_port = 35670;
    string m_gdb;

public:
    CommandLineParser();
    virtual ~CommandLineParser();

    void Parse(int argc, char** argv);

    void SetGdb(const string& gdb) { this->m_gdb = gdb; }
    void SetHost(const string& host) { this->m_host = host; }
    void SetPort(int port) { this->m_port = port; }
    void SetVerbose(bool verbose) { this->m_verbose = verbose; }
    const string& GetGdb() const { return m_gdb; }
    const string& GetHost() const { return m_host; }
    int GetPort() const { return m_port; }
    bool IsVerbose() const { return m_verbose; }
};
#endif // COMMANDLINEPARSER_HPP
