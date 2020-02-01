#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <string>

using namespace std;

namespace dap
{
class Exception
{
    string m_what;
public:
    Exception(const string& what);
    virtual ~Exception();
    
    const string& What() const;
};

};     // namespace dap
#endif // EXCEPTION_HPP
