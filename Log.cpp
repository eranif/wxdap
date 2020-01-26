#include "Log.hpp"
#include "dap/StringUtils.hpp"
#include <chrono>
#include <sstream>
#include <sys/time.h>

int Log::m_verbosity = Log::Error;
string Log::m_logfile;

Log::Log(int requestedVerbo)
    : _requestedLogLevel(requestedVerbo)
    , m_fp(nullptr)
{
}

Log::~Log()
{
    // flush any content that remain
    Flush();
}

void Log::AddLogLine(const string& msg, int verbosity)
{
    if(msg.empty()) {
        return;
    }
    if((m_verbosity >= verbosity)) {
        string formattedMsg = Prefix(verbosity);
        m_buffer << formattedMsg << " " << msg;
        m_buffer << "\n";
    }
}

void Log::SetVerbosity(int level)
{
    if(level > Log::Warning) {
        LOG_SYSTEM() << Log::GetVerbosityAsString(level) << string("");
    }
    m_verbosity = level;
}

int Log::GetVerbosityAsNumber(const string& verbosity)
{
    if(verbosity == "Debug") {
        return Log::Dbg;

    } else if(verbosity == "Error") {
        return Log::Error;

    } else if(verbosity == "Warning") {
        return Log::Warning;

    } else if(verbosity == "System") {
        return Log::System;

    } else if(verbosity == "Developer") {
        return Log::Developer;

    } else {
        return Log::Error;
    }
}

string Log::GetVerbosityAsString(int verbosity)
{
    switch(verbosity) {
    case Log::Dbg:
        return "Debug";

    case Log::Error:
        return "Error";

    case Log::Warning:
        return "Warning";

    case Log::Developer:
        return "Developer";

    default:
        return "Error";
    }
}

void Log::SetVerbosity(const string& verbosity) { SetVerbosity(GetVerbosityAsNumber(verbosity)); }

void Log::OpenLog(const string& fullpath, int verbosity)
{
    m_logfile = fullpath;
    m_verbosity = verbosity;
}

void Log::Flush()
{
    if(m_buffer.str().empty()) {
        return;
    }
    if(!m_fp) {
        m_fp = fopen(m_logfile.c_str(), "a+");
    }

    if(m_fp) {
        fprintf(m_fp, "%s\n", m_buffer.str().c_str());
        fclose(m_fp);
        m_fp = nullptr;
    }
    m_buffer.clear();
}

string Log::Prefix(int verbosity)
{
    if(verbosity <= m_verbosity) {
        stringstream prefix;
        timeval tim;
        gettimeofday(&tim, NULL);
        auto start = chrono::system_clock::now();
        auto as_time_t = chrono::system_clock::to_time_t(start);
        string timeString = ctime(&as_time_t);
        StringUtils::Trim(timeString);

        prefix << "[" << timeString;
        switch(verbosity) {
        case System:
            prefix << " SYS]";
            break;

        case Error:
            prefix << " ERR]";
            break;

        case Warning:
            prefix << " WRN]";
            break;

        case Dbg:
            prefix << " DBG]";
            break;

        case Developer:
            prefix << " DVL]";
            break;
        }

        prefix << " ";
        string p = prefix.str();
        return p;
    } else {
        return "";
    }
}
