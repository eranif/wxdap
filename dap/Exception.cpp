#include "Exception.hpp"

#include "StringUtils.hpp"

dap::Exception::Exception(const wxString& what)
    : m_what(what)
{
    StringUtils::Trim(m_what);
}

dap::Exception::~Exception() {}

const wxString& dap::Exception::What() const { return m_what; }
