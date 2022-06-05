#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <sstream>
#include <string>
#include <vector>
#include <wx/string.h>

// for some obscure reason, to_string() does not accept wxString
// we add one here
namespace std
{
const wxString& to_string(const wxString& str);
};

#define UNUSED(x) ((void)x)

class StringUtils
{
protected:
    static char** BuildArgv(const wxString& str, int& argc);
    static void FreeArgv(char** argv, int argc);

public:
    /// Right trim
    static wxString& Rtrim(wxString& str);

    /// Left trim
    static wxString& Ltrim(wxString& str);

    /// Both left + right trim
    static wxString& Trim(wxString& str);

    /// Gets all characters before the first occurrence of ch.
    /// Returns the whole wxString if ch is not found.
    static wxString BeforeFirst(const wxString& str, char ch);

    /// Gets all the characters after the first occurrence of ch.
    /// Returns the empty wxString if ch is not found.
    static wxString AfterFirst(const wxString& str, char ch);

    /// Check if wxString starts with a given prefix
    static bool StartsWith(const wxString& str, const wxString& prefix);

    /// Split a wxString
    static std::vector<wxString> Split(const wxString& str, char ch = '\n');

    /// Convert to wxString to uppercase
    static wxString ToUpper(const wxString& str);

    /// Split command line into array
    static std::vector<wxString> BuildArgv(const wxString& str);

    /// Convert file's path to native path
    /// this function also removes double \\ or //
    static wxString& ToNativePath(wxString& path);

    /// Const version
    static wxString ToNativePath(const wxString& path);

    /// Convert file's path to UNIX slashes
    static wxString& ToUnixPath(wxString& path);

    /// Const version
    static wxString ToUnixPath(const wxString& path);

    /// Wrap wxString with quotes if needed
    static wxString& WrapWithQuotes(wxString& str);

    /// Wrap wxString with quotes if needed
    static wxString WrapWithQuotes(const wxString& str);
};

template <typename T>
wxString& operator<<(wxString& str, const T& t)
{
    str.append(to_string(t));
    return str;
}

#endif // STRINGUTILS_H
