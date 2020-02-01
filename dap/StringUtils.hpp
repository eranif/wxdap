#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <vector>

using namespace std;
#define UNUSED(x) ((void)x)

class StringUtils
{
protected:
    static char** BuildArgv(const string& str, int& argc);
    static void FreeArgv(char** argv, int argc);
    
public:
    /// Right trim
    static wstring& Rtrim(wstring& str);
    static string& Rtrim(string& str);
    
    /// Left trim
    static wstring& Ltrim(wstring& str);
    static string& Ltrim(string& str);

    /// Both left + right trim
    static wstring& Trim(wstring& str);
    static string& Trim(string& str);

    /// Gets all characters before the first occurrence of ch.
    /// Returns the whole string if ch is not found.
    static string BeforeFirst(const string& str, char ch);

    /// Gets all the characters after the first occurrence of ch.
    /// Returns the empty string if ch is not found.
    static string AfterFirst(const string& str, char ch);
    
    /// Check if string starts with a given prefix
    static bool StartsWith(const string& str, const string& prefix);
    
    /// Split a string
    static vector<string> Split(const string& str, char ch = '\n');
    
    /// Convert to string to uppercase
    static string ToUpper(const string& str);
    
    /// Split command line into array
    static vector<string> BuildArgv(const string& str);
    
    /// Convert file's path to native path
    /// this function also removes double \\ or //
    static string& ToNativePath(string& path);
    
    /// Convert file's path to UNIX slashes
    static string& ToUnixPath(string& path);
    
    /// Wrap string with quotes if needed
    static string& WrapWithQuotes(string& str);
    
    /// Wrap string with quotes if needed
    static string WrapWithQuotes(const string& str);
};

#endif // STRINGUTILS_H
