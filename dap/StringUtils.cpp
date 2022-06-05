#include "StringUtils.hpp"
#include <codecvt>
#include <cstring>
#include <locale>
#include <sstream>

std::wstring& StringUtils::Rtrim(std::wstring& str)
{
    str.erase(str.find_last_not_of(L" \n\r\t") + 1);
    return str;
}

std::string& StringUtils::Rtrim(std::string& str)
{
    str.erase(str.find_last_not_of(" \n\r\t") + 1);
    return str;
}

std::wstring& StringUtils::Ltrim(std::wstring& str)
{
    str.erase(0, str.find_first_not_of(L" \n\r\t"));
    return str;
}

std::string& StringUtils::Ltrim(std::string& str)
{
    str.erase(0, str.find_first_not_of(" \n\r\t"));
    return str;
}

std::wstring& StringUtils::Trim(std::wstring& str)
{
    str.erase(0, str.find_first_not_of(L" \n\r\t"));
    str.erase(str.find_last_not_of(L" \n\r\t") + 1);
    return str;
}

std::string& StringUtils::Trim(std::string& str)
{
    str.erase(0, str.find_first_not_of(" \n\r\t"));
    str.erase(str.find_last_not_of(" \n\r\t") + 1);
    return str;
}

std::string StringUtils::BeforeFirst(const std::string& str, char ch)
{
    size_t where = str.find(ch);
    if(where == std::string::npos) {
        return str;
    }
    return str.substr(0, where);
}

std::string StringUtils::AfterFirst(const std::string& str, char ch)
{
    size_t where = str.find(ch);
    if(where == std::string::npos) {
        return "";
    }
    return str.substr(where + 1);
}

std::vector<std::string> StringUtils::Split(const std::string& str, char ch)
{
    std::vector<std::string> v;
    std::istringstream iss(str);
    std::string token;
    while(getline(iss, token, ch)) {
        v.push_back(token);
    }
    return v;
}

std::string StringUtils::ToUpper(const std::string& str)
{
    std::string upper;
    for(auto ch : str) {
        upper.append(1, toupper(ch));
    }
    return upper;
}
#define ARGV_STATE_NORMAL 0
#define ARGV_STATE_DQUOTE 1
#define ARGV_STATE_SQUOTE 2
#define ARGV_STATE_ESCAPE 3
#define ARGV_STATE_BACKTICK 4
#define PUSH_CURTOKEN()                \
    {                                  \
        if(!curstr.str().empty()) {    \
            A.push_back(curstr.str()); \
            curstr = {};               \
        }                              \
    }

#define CHANGE_STATE(new_state) \
    {                           \
        prev_state = state;     \
        state = new_state;      \
    }

#define RESTORE_STATE()                 \
    {                                   \
        state = prev_state;             \
        prev_state = ARGV_STATE_NORMAL; \
    }

char** StringUtils::BuildArgv(const std::string& str, int& argc)
{
    std::vector<std::string> A;
    int state = ARGV_STATE_NORMAL;
    int prev_state = ARGV_STATE_NORMAL;
    std::stringstream curstr;
    for(char ch : str) {
        switch(state) {
        case ARGV_STATE_NORMAL: {
            switch(ch) {
            case ' ':
            case '\t':
                PUSH_CURTOKEN();
                break;
            case '\'':
                CHANGE_STATE(ARGV_STATE_SQUOTE);
                curstr << ch;
                break;
            case '"':
                CHANGE_STATE(ARGV_STATE_DQUOTE);
                curstr << ch;
                break;
            case '`':
                CHANGE_STATE(ARGV_STATE_BACKTICK);
                curstr << ch;
                break;
            default:
                curstr << ch;
                break;
            }
        } break;
        case ARGV_STATE_ESCAPE: {
            if(prev_state == ARGV_STATE_DQUOTE) {
                switch(ch) {
                case '"':
                    curstr << "\"";
                    RESTORE_STATE();
                    break;
                default:
                    curstr << "\\" << ch;
                    RESTORE_STATE();
                    break;
                }
            } else if(prev_state == ARGV_STATE_BACKTICK) {
                switch(ch) {
                case '`':
                    curstr << "`";
                    RESTORE_STATE();
                    break;
                default:
                    curstr << "\\" << ch;
                    RESTORE_STATE();
                    break;
                }
            } else { // single quote
                switch(ch) {
                case '\'':
                    curstr << "'";
                    RESTORE_STATE();
                    break;
                default:
                    curstr << "\\" << ch;
                    RESTORE_STATE();
                    break;
                }
            }
        } break;
        case ARGV_STATE_DQUOTE: {
            switch(ch) {
            case '\\':
                CHANGE_STATE(ARGV_STATE_ESCAPE);
                break;
            case '"':
                curstr << ch;
                RESTORE_STATE();
                break;
            default:
                curstr << ch;
                break;
            }
        } break;
        case ARGV_STATE_SQUOTE: {
            switch(ch) {
            case '\\':
                CHANGE_STATE(ARGV_STATE_ESCAPE);
                break;
            case '\'':
                curstr << ch;
                RESTORE_STATE();
                break;
            default:
                curstr << ch;
                break;
            }
        } break;
        case ARGV_STATE_BACKTICK: {
            switch(ch) {
            case '\\':
                CHANGE_STATE(ARGV_STATE_ESCAPE);
                break;
            case '`':
                curstr << ch;
                RESTORE_STATE();
                break;
            default:
                curstr << ch;
                break;
            }
        } break;
        }
    }

    if(!curstr.str().empty()) {
        A.push_back(curstr.str());
    }

    if(A.empty()) {
        return nullptr;
    }

    char** argv = new char*[A.size() + 1];
    argv[A.size()] = NULL;
    for(size_t i = 0; i < A.size(); ++i) {
        argv[i] = strdup(A[i].c_str());
    }
    argc = (int)A.size();
    return argv;
}

void StringUtils::FreeArgv(char** argv, int argc)
{
    for(int i = 0; i < argc; ++i) {
        free(argv[i]);
    }
    delete[] argv;
}

std::vector<std::string> StringUtils::BuildArgv(const std::string& str)
{
    int argc = 0;
    char** argv = BuildArgv(str, argc);
    std::vector<std::string> arrArgv;
    for(int i = 0; i < argc; ++i) {
        arrArgv.push_back(argv[i]);
    }
    FreeArgv(argv, argc);

    for(std::string& s : arrArgv) {
        if((s.length() > 1) && (s[0] == '"') && (s.back() == '"')) {
            s.pop_back();
            s.erase(0, 1);
        }
    }
    return arrArgv;
}

#ifdef __WIN32
#define PATH_SEP '\\'
#define SOURCE_SEP '/'
#else
#define PATH_SEP '/'
#define SOURCE_SEP '\\'
#endif

static std::string& ConvertSlashes(std::string& path, char source, char target)
{
    char last_char = 0;
    std::string tmp;
    tmp.reserve(path.length());
    for(char& ch : path) {
        if(ch == source) {
            ch = target;
        }
        if(ch == target && last_char == target) {
            // Skip it
        } else {
            tmp.append(1, ch);
        }
        last_char = ch;
    }
    path = tmp;
    return path;
}
const std::string& std::to_string(const std::string& str) { return str; }

std::string& StringUtils::ToNativePath(std::string& path) { return ConvertSlashes(path, SOURCE_SEP, PATH_SEP); }

std::string& StringUtils::ToUnixPath(std::string& path) { return ConvertSlashes(path, '\\', '/'); }

std::string StringUtils::ToUnixPath(const std::string& path)
{
    std::string tmppath = path;
    tmppath = ConvertSlashes(tmppath, '\\', '/');
    return tmppath;
}

std::string StringUtils::ToNativePath(const std::string& path)
{
    std::string tmppath = path;
    tmppath = ConvertSlashes(tmppath, SOURCE_SEP, PATH_SEP);
    return tmppath;
}

std::string& StringUtils::WrapWithQuotes(std::string& str)
{
    if(str.empty()) {
        return str;
    }
    if(str.find(' ') == std::string::npos) {
        return str;
    }
    str.insert(str.begin(), '"');
    str.append(1, '"');
    return str;
}

std::string StringUtils::WrapWithQuotes(const std::string& str)
{
    if(str.empty()) {
        return str;
    }
    if(str.find(' ') == std::string::npos) {
        return str;
    }
    std::string tmpstr = str;
    tmpstr.insert(tmpstr.begin(), '"');
    tmpstr.append(1, '"');
    return tmpstr;
}

bool StringUtils::StartsWith(const std::string& str, const std::string& prefix)
{
    if(str.length() < prefix.length()) {
        return false;
    }

    for(size_t i = 0; i < prefix.length(); ++i) {
        if(str[i] != prefix[i]) {
            return false;
        }
    }
    return true;
}
