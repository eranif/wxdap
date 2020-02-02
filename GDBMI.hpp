#ifndef GDBMI_HPP
#define GDBMI_HPP

#include "dap/dap.hpp"
#include <string>
#include <vector>

using namespace std;
class GDBMI
{
public:
    struct Breakpoint {
        string number;
        string file;
        string fullname;
        bool enabled = true;
        string func;
        string type;
        string line;
        typedef vector<Breakpoint> Vec_t;
    };

protected:
    static vector<string> SplitToBlocksCurly(const string& buffer);
    static vector<pair<string, string>> SplitToKeyValues(const string& buffer);
    static dap::Breakpoint DoParseBreakpoint(const string& block);

public:
    /**
     * @brief parse the output of the command -break-list and return list of breakpoints
     */
    static vector<dap::Breakpoint> ParseBreakpoints(const string& gdbOutput);

    /**
     * @brief parse -break-insert output
     */
    static dap::Breakpoint ParseBreakpoint(const string& gdbOutput);
};

#endif // GDBMI_HPP
