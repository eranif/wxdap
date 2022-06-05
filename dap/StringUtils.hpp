#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <sstream>
#include <string>
#include <vector>

// for some obscure reason, to_string() does not accept std::string
// we add one here
namespace std
{
const std::string& to_string(const std::string& str);
};

#define UNUSED(x) ((void)x)

class StringUtils
{
protected:
    static char** BuildArgv(const std::string& str, int& argc);
    static void FreeArgv(char** argv, int argc);

public:
    /// Right trim
    static std::wstring& Rtrim(std::wstring& str);
    static std::string& Rtrim(std::string& str);

    /// Left trim
    static std::wstring& Ltrim(std::wstring& str);
    static std::string& Ltrim(std::string& str);

    /// Both left + right trim
    static std::wstring& Trim(std::wstring& str);
    static std::string& Trim(std::string& str);

    /// Gets all characters before the first occurrence of ch.
    /// Returns the whole std::string if ch is not found.
    static std::string BeforeFirst(const std::string& str, char ch);

    /// Gets all the characters after the first occurrence of ch.
    /// Returns the empty std::string if ch is not found.
    static std::string AfterFirst(const std::string& str, char ch);

    /// Check if std::string starts with a given prefix
    static bool StartsWith(const std::string& str, const std::string& prefix);

    /// Split a std::string
    static std::vector<std::string> Split(const std::string& str, char ch = '\n');

    /// Convert to std::string to uppercase
    static std::string ToUpper(const std::string& str);

    /// Split command line into array
    static std::vector<std::string> BuildArgv(const std::string& str);

    /// Convert file's path to native path
    /// this function also removes double \\ or //
    static std::string& ToNativePath(std::string& path);

    /// Const version
    static std::string ToNativePath(const std::string& path);

    /// Convert file's path to UNIX slashes
    static std::string& ToUnixPath(std::string& path);

    /// Const version
    static std::string ToUnixPath(const std::string& path);

    /// Wrap std::string with quotes if needed
    static std::string& WrapWithQuotes(std::string& str);

    /// Wrap std::string with quotes if needed
    static std::string WrapWithQuotes(const std::string& str);
};

template <typename T>
std::string& operator<<(std::string& str, const T& t)
{
    str.append(to_string(t));
    return str;
}

#endif // STRINGUTILS_H
