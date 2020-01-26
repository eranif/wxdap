#ifndef LOG_HPP
#define LOG_HPP

#include <sstream>
#include <string>
#include <vector>

// manipulator function
class Log;
typedef Log& (*LogFunction)(Log&);
using namespace std;
class Log
{
public:
    enum { System = -1, Error = 0, Warning = 1, Dbg = 2, Developer = 3 };

protected:
    static int m_verbosity;
    static string m_logfile;
    int _requestedLogLevel;
    FILE* m_fp;
    stringstream m_buffer;

protected:
    static string GetCurrentThreadName();

public:
    Log(int requestedVerbo);
    ~Log();

    stringstream& GetStream() { return m_buffer; }

    Log& SetRequestedLogLevel(int level)
    {
        _requestedLogLevel = level;
        return *this;
    }

    int GetRequestedLogLevel() const { return _requestedLogLevel; }

    /**
     * @brief create log entry prefix
     */
    static string Prefix(int verbosity);

    void AddLogLine(const string& msg, int verbosity);
    static void SetVerbosity(int level);

    // Set the verbosity as string
    static void SetVerbosity(const string& verbosity);

    /**
     * @brief open the log file
     */
    static void OpenLog(const string& fullpath, int verbosity);
    
    // Various util methods
    static string GetVerbosityAsString(int verbosity);
    static int GetVerbosityAsNumber(const string& verbosity);

    template <typename T>
    stringstream& operator<<(const T& str)
    {
        Append(str, GetRequestedLogLevel());
        return GetStream();
    }

    /**
     * @brief append any type to the buffer, take log level into consideration
     */
    template <typename T>
    stringstream& Append(const T& elem, int level)
    {
        if(level > m_verbosity) {
            return m_buffer;
        }
        if(!m_buffer.str().empty()) {
            m_buffer << " ";
        }
        m_buffer << elem;
        return m_buffer;
    }

    /**
     * @brief flush the logger content
     */
    void Flush();
};

inline stringstream& clEndl(Log& d)
{
    d.Flush();
    return d.GetStream();
}

// New API
#define LOG_DEBUG() Log(Log::Dbg) << Log::Prefix(Log::Dbg)
#define LOG_DEBUG1() Log(Log::Developer) << Log::Prefix(Log::Developer)
#define LOG_ERROR() Log(Log::Error) << Log::Prefix(Log::Error)
#define LOG_WARNING() Log(Log::Warning) << Log::Prefix(Log::Warning)
#define LOG_SYSTEM() Log(Log::System) << Log::Prefix(Log::System)

// A replacement for wxLogMessage
#define clLogMessage(msg) clDEBUG() << msg

#endif // LOG_HPP
