#include "Exception.hpp"
#include "StringUtils.hpp"

dap::Exception::Exception(const string& what)
    : m_what(what)
{
    StringUtils::Trim(m_what);
}

dap::Exception::~Exception() {}

const string& dap::Exception::What() const { return m_what; }
