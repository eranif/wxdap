#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <string>

namespace dap
{
class Exception
{
    std::string m_what;

public:
    Exception(const std::string& what);
    virtual ~Exception();

    const std::string& What() const;
};

};     // namespace dap
#endif // EXCEPTION_HPP
