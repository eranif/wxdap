#ifndef GDBMI_HPP
#define GDBMI_HPP

#include "dap/dap.hpp"
#include <string>
#include <vector>
#include <unordered_map>

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

public:
    enum eStoppedReason {
        kUnknown = -1,
        kBreakpointHit,
    };

protected:
    static vector<string> SplitToBlocksCurly(const string& buffer);
    static unordered_map<string, string> SplitToKeyValues(const string& buffer);
    static dap::Breakpoint DoParseBreakpoint(const string& block);
    static dap::StackFrame DoParseStackFrame(const string& block);

public:
    /**
     * @brief parse the output of the command -break-list and return list of breakpoints
     */
    static vector<dap::Breakpoint> ParseBreakpoints(const string& gdbOutput);

    /**
     * @brief parse -break-insert output
     */
    static dap::Breakpoint ParseBreakpoint(const string& gdbOutput);

    /**
     * @brief parse frame
     */
    static dap::StackFrame ParseStackFrame(const string& gdbOutput);
    /**
     * @brief return the "stopped" reason
     */
    static eStoppedReason ParseStoppedReason(const string& gdbOutput);
};

#endif // GDBMI_HPP
