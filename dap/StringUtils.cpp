#include "StringUtils.hpp"

#include <codecvt>
#include <cstring>
#include <locale>
#include <sstream>
#include <wx/arrstr.h>
#include <wx/tokenzr.h>

wxString& DapStringUtils::Rtrim(wxString& str) { return str.Trim(); }

wxString& DapStringUtils::Ltrim(wxString& str) { return str.Trim(false); }

wxString& DapStringUtils::Trim(wxString& str) { return str.Trim().Trim(false); }

wxString DapStringUtils::BeforeFirst(const wxString& str, char ch)
{
    size_t where = str.find(ch);
    if(where == wxString::npos) {
        return str;
    }
    return str.substr(0, where);
}

wxString DapStringUtils::AfterFirst(const wxString& str, char ch)
{
    size_t where = str.find(ch);
    if(where == wxString::npos) {
        return "";
    }
    return str.substr(where + 1);
}

std::vector<wxString> DapStringUtils::Split(const wxString& str, char ch)
{
    std::vector<wxString> v;
    auto arr = ::wxStringTokenize(str, wxString() << ch, wxTOKEN_STRTOK);
    v.reserve(arr.size());
    v.insert(v.end(), arr.begin(), arr.end());
    return v;
}

wxString DapStringUtils::ToUpper(const wxString& str) { return str.Upper(); }

#define ARGV_STATE_NORMAL 0
#define ARGV_STATE_DQUOTE 1
#define ARGV_STATE_SQUOTE 2
#define ARGV_STATE_ESCAPE 3
#define ARGV_STATE_BACKTICK 4
#define PUSH_CURTOKEN()          \
    {                            \
        if(!curstr.empty()) {    \
            A.push_back(curstr); \
            curstr.clear();      \
        }                        \
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

char** DapStringUtils::BuildArgv(const wxString& str, int& argc)
{
    std::vector<wxString> A;
    int state = ARGV_STATE_NORMAL;
    int prev_state = ARGV_STATE_NORMAL;
    wxString curstr;
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

    if(!curstr.empty()) {
        A.push_back(curstr);
        curstr.clear();
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

void DapStringUtils::FreeArgv(char** argv, int argc)
{
    for(int i = 0; i < argc; ++i) {
        free(argv[i]);
    }
    delete[] argv;
}

std::vector<wxString> DapStringUtils::BuildArgv(const wxString& str)
{
    int argc = 0;
    char** argv = BuildArgv(str, argc);
    std::vector<wxString> arrArgv;
    for(int i = 0; i < argc; ++i) {
        arrArgv.push_back(argv[i]);
    }
    FreeArgv(argv, argc);

    for(wxString& s : arrArgv) {
        if((s.length() > 1) && (s[0] == '"') && (s.Last() == '"')) {
            s.RemoveLast();
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

static wxString& ConvertSlashes(wxString& path, char source, char target)
{
    char last_char = 0;
    wxString tmp;
    tmp.reserve(path.length());
    for(wxChar ch : path) {
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
const wxString& std::to_string(const wxString& str) { return str; }

wxString& DapStringUtils::ToNativePath(wxString& path) { return ConvertSlashes(path, SOURCE_SEP, PATH_SEP); }

wxString& DapStringUtils::ToUnixPath(wxString& path) { return ConvertSlashes(path, '\\', '/'); }

wxString DapStringUtils::ToUnixPath(const wxString& path)
{
    wxString tmppath = path;
    tmppath = ConvertSlashes(tmppath, '\\', '/');
    return tmppath;
}

wxString DapStringUtils::ToNativePath(const wxString& path)
{
    wxString tmppath = path;
    tmppath = ConvertSlashes(tmppath, SOURCE_SEP, PATH_SEP);
    return tmppath;
}

wxString& DapStringUtils::WrapWithQuotes(wxString& str)
{
    if(str.empty()) {
        return str;
    }
    if(str.find(' ') == wxString::npos) {
        return str;
    }
    str.insert(str.begin(), '"');
    str.append(1, '"');
    return str;
}

wxString DapStringUtils::WrapWithQuotes(const wxString& str)
{
    if(str.empty()) {
        return str;
    }
    if(str.find(' ') == wxString::npos) {
        return str;
    }
    wxString tmpstr = str;
    tmpstr.insert(tmpstr.begin(), '"');
    tmpstr.append(1, '"');
    return tmpstr;
}

bool DapStringUtils::StartsWith(const wxString& str, const wxString& prefix)
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
