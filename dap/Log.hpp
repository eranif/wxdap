#ifndef LOG_HPP
#define LOG_HPP

#include "dap_exports.hpp"
#include <string>
#include <vector>
#include <wx/string.h>

// manipulator function
class Log;
namespace dap
{
class WXDLLIMPEXP_DAP Log
{
public:
    enum eLogVerbosity { System = -1, Error, Warning, Info, Dbg, Developer };

protected:
    static int m_verbosity;
    static wxString m_logfile;
    static bool m_useStdout;
    int m_requestedLogLevel = Error;
    FILE* m_fp = nullptr;
    wxString m_buffer;

protected:
    static const wxString& GetColour(int verbo);
    static const wxString& GetColourEnd();

public:
    Log(int requestedVerbo);
    ~Log();

    /**
     * @brief return the internal stream buffer
     */
    wxString& GetStream() { return m_buffer; }

    Log& SetRequestedLogLevel(int level)
    {
        m_requestedLogLevel = level;
        return *this;
    }

    int GetRequestedLogLevel() const { return m_requestedLogLevel; }

    /**
     * @brief create log entry prefix
     */
    static wxString Prefix(int verbosity);

    void AddLogLine(const wxString& msg, int verbosity);
    static void SetVerbosity(int level);

    // Set the verbosity as wxString
    static void SetVerbosity(const wxString& verbosity);

    /**
     * @brief open the log file
     */
    static void OpenLog(const wxString& fullpath, int verbosity);
    /**
     * @brief open stdout as the log stream
     */
    static void OpenStdout(int verbosity);

    // Various util methods
    static wxString GetVerbosityAsString(int verbosity);
    static int GetVerbosityAsNumber(const wxString& verbosity);

    inline Log& Append(const std::vector<wxString>& arr, int level)
    {
        if(arr.empty()) {
            return *this;
        }
        wxString str;
        str += "[";
        for(auto s : arr) {
            str += s;
            str += ", ";
        }
        str.RemoveLast();
        str.RemoveLast();
        str += "]";
        Append(str, GetRequestedLogLevel());
        return *this;
    }

    inline Log& operator<<(const wxString& str)
    {
        if(GetRequestedLogLevel() > m_verbosity) {
            return *this;
        }
        if(!m_buffer.empty()) {
            m_buffer << " ";
        }
        m_buffer << str;
        return *this;
    }

    /**
     * @brief append any type to the buffer, take log level into consideration
     */
    template <typename T>
    Log& Append(const T& elem, int level)
    {
        if(level > m_verbosity) {
            return *this;
        }
        if(!m_buffer.empty()) {
            m_buffer << " ";
        }
        m_buffer << elem;
        return *this;
    }

    /**
     * @brief flush the logger content
     */
    void Flush();
};

inline Log& endl(Log& d)
{
    d.Flush();
    return d;
}

typedef Log& (*LogFunction)(Log&);
inline Log& operator<<(Log& logger, const LogFunction&)
{
    logger.Flush();
    return logger;
}

template <typename T>
Log& operator<<(Log& logger, const T& obj)
{
    logger.Append(obj, logger.GetRequestedLogLevel());
    return logger;
}

// New API
#define LOG_DEBUG() dap::Log(dap::Log::Dbg) << dap::Log::Prefix(dap::Log::Dbg)
#define LOG_DEBUG1() dap::Log(dap::Log::Developer) << dap::Log::Prefix(dap::Log::Developer)
#define LOG_ERROR() dap::Log(dap::Log::Error) << dap::Log::Prefix(dap::Log::Error)
#define LOG_WARNING() dap::Log(dap::Log::Warning) << dap::Log::Prefix(dap::Log::Warning)
#define LOG_SYSTEM() dap::Log(dap::Log::System) << dap::Log::Prefix(dap::Log::System)
#define LOG_INFO() dap::Log(dap::Log::Info) << dap::Log::Prefix(dap::Log::Info)
};     // namespace dap
#endif // LOG_HPP
