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
    
    /// Split a string
    static vector<string> Split(const string& str, char ch = '\n');
    
    /// Convert to string to uppercase
    static string ToUpper(const string& str);
    
    /// Split command line into array
    static vector<string> BuildArgv(const string& str);
};

#endif // STRINGUTILS_H
