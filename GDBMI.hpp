#ifndef GDBMI_HPP
#define GDBMI_HPP

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
    vector<string> SplitToBlocksCurly(string& buffer);
    vector<pair<string, string>> SplitToKeyValues(string& buffer);

public:
    GDBMI();
    virtual ~GDBMI();

    /**
     * @brief parse the output of the command -break-list and return list of breakpoints
     */
    Breakpoint::Vec_t ParseBreakpoints(const string& gdbOutput);
};

#endif // GDBMI_HPP
