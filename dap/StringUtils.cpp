#include "StringUtils.hpp"
#include <codecvt>
#include <locale>
#include <sstream>

wstring& StringUtils::Rtrim(wstring& str)
{
    str.erase(str.find_last_not_of(L" \n\r\t") + 1);
    return str;
}

string& StringUtils::Rtrim(string& str)
{
    str.erase(str.find_last_not_of(" \n\r\t") + 1);
    return str;
}

wstring& StringUtils::Ltrim(wstring& str)
{
    str.erase(0, str.find_first_not_of(L" \n\r\t"));
    return str;
}

string& StringUtils::Ltrim(string& str)
{
    str.erase(0, str.find_first_not_of(" \n\r\t"));
    return str;
}

wstring& StringUtils::Trim(wstring& str)
{
    str.erase(0, str.find_first_not_of(L" \n\r\t"));
    str.erase(str.find_last_not_of(L" \n\r\t") + 1);
    return str;
}

string& StringUtils::Trim(string& str)
{
    str.erase(0, str.find_first_not_of(" \n\r\t"));
    str.erase(str.find_last_not_of(" \n\r\t") + 1);
    return str;
}

string StringUtils::BeforeFirst(const string& str, char ch)
{
    size_t where = str.find(ch);
    if(where == string::npos) {
        return str;
    }
    return str.substr(0, where);
}

string StringUtils::AfterFirst(const string& str, char ch)
{
    size_t where = str.find(ch);
    if(where == string::npos) {
        return "";
    }
    return str.substr(where + 1);
}

vector<string> StringUtils::Split(const string& str, char ch)
{
    vector<string> v;
    istringstream iss(str);
    string token;
    while(getline(iss, token, ch)) {
        v.push_back(token);
    }
    return v;
}
