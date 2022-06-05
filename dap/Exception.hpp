#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <wx/string.h>

namespace dap
{
class Exception
{
    wxString m_what;

public:
    Exception(const wxString& what);
    virtual ~Exception();

    const wxString& What() const;
};

};     // namespace dap
#endif // EXCEPTION_HPP
