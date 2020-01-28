#include "StringUtils.hpp"
#include <codecvt>
#include <cstring>
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

string StringUtils::ToUpper(const string& str)
{
    string upper;
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
            curstr = stringstream();   \
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

char** StringUtils::BuildArgv(const string& str, int& argc)
{
    vector<string> A;
    int state = ARGV_STATE_NORMAL;
    int prev_state = ARGV_STATE_NORMAL;
    stringstream curstr;
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

vector<string> StringUtils::BuildArgv(const string& str)
{
    int argc = 0;
    char** argv = BuildArgv(str, argc);
    vector<string> arrArgv;
    for(int i = 0; i < argc; ++i) {
        arrArgv.push_back(argv[i]);
    }
    FreeArgv(argv, argc);

    for(string& s : arrArgv) {
        if((s.length() > 1) && (s[0] == '"') && (s.back() == '"')) {
            s.pop_back();
            s.erase(0, 1);
        }
    }
    return arrArgv;
}
