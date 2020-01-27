#include "Log.hpp"
#include "dap/StringUtils.hpp"
#include <chrono>
#include <sstream>
#include <sys/time.h>

int Log::m_verbosity = Log::Error;
string Log::m_logfile;
bool Log::m_useStdout = false;

static const char ESCAPE = 0x1B;
static const string GREEN = "\e[32m";
static const string RED = "\e[31m";
static const string YELLOW = "\e[93m";
static const string CYAN = "\e[96m";
static const string COLOUR_END = "\e[0m";

Log::Log(int requestedVerbo)
    : m_requestedLogLevel(requestedVerbo)
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
    m_useStdout = false;
}

void Log::OpenStdout(int verbosity)
{
    m_logfile.clear();
    m_useStdout = true;
    m_verbosity = verbosity;
}

void Log::Flush()
{
    if(m_buffer.str().empty()) {
        return;
    }

    if(m_useStdout) {
        m_fp = stdout;
    }

    if(!m_fp) {
        m_fp = fopen(m_logfile.c_str(), "a+");
    }

    if(m_fp) {
        fprintf(m_fp, "%s\n", m_buffer.str().c_str());
        // Dont close stdout
        if(!m_useStdout) {
            fclose(m_fp);
        }
        m_fp = nullptr;
    }
    m_buffer.clear();
}

string Log::Prefix(int verbosity)
{
    if(verbosity <= m_verbosity) {
        timeval tim;
        gettimeofday(&tim, NULL);
        auto start = chrono::system_clock::now();
        auto as_time_t = chrono::system_clock::to_time_t(start);
        string timeString = ctime(&as_time_t);
        StringUtils::Trim(timeString);

        stringstream prefix;
        switch(verbosity) {
        case System:
            prefix << CYAN << "[" << timeString << " SYS]" << COLOUR_END;
            break;

        case Error:
            prefix << RED << "[" << timeString << " ERR]" << COLOUR_END;
            break;

        case Warning:
            prefix << YELLOW << "[" << timeString << " WRN]" << COLOUR_END;
            break;

        case Dbg:
            prefix << "[" << timeString << " DBG]";
            break;

        case Developer:
            prefix << "[" << timeString << " DVL]";
            break;
        }

        prefix << " ";
        string p = prefix.str();
        return p;
    } else {
        return "";
    }
}
