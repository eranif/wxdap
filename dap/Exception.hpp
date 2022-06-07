#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include "dap_exports.hpp"
#include <wx/string.h>

namespace dap
{
class WXDLLIMPEXP_DAP Exception
{
    wxString m_what;

public:
    Exception(const wxString& what);
    virtual ~Exception();

    const wxString& What() const;
};

};     // namespace dap
#endif // EXCEPTION_HPP
