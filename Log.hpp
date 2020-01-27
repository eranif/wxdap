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
    enum eLogVerbosity { System = -1, Error, Warning, Info, Dbg, Developer };

protected:
    static int m_verbosity;
    static string m_logfile;
    static bool m_useStdout;
    int m_requestedLogLevel = Error;
    FILE* m_fp = nullptr;
    stringstream m_buffer;

protected:
    static const string& GetColour(int verbo);
    static const string& GetColourEnd();

public:
    Log(int requestedVerbo);
    ~Log();

    /**
     * @brief return the internal stream buffer
     */
    stringstream& GetStream() { return m_buffer; }

    Log& SetRequestedLogLevel(int level)
    {
        m_requestedLogLevel = level;
        return *this;
    }

    int GetRequestedLogLevel() const { return m_requestedLogLevel; }

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
    /**
     * @brief open stdout as the log stream
     */
    static void OpenStdout(int verbosity);

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
#define LOG_INFO() Log(Log::Info) << Log::Prefix(Log::Info)

#endif // LOG_HPP
