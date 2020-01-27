#include "Log.hpp"
#include "dap/StringUtils.hpp"
#include <chrono>
#include <sstream>
#include <sys/time.h>

int Log::m_verbosity = Log::Error;
string Log::m_logfile;
bool Log::m_useStdout = false;

static const string GREEN = "\x1b[32m";
static const string RED = "\x1b[31m";
static const string YELLOW = "\x1b[93m";
static const string CYAN = "\x1b[96m";
static const string WHITE = "\x1b[37m";
static const string COLOUR_END = "\x1b[0m";
static const string EMPTY_STR = "";

// Needed for Windows
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
// Some old MinGW/CYGWIN distributions don't define this:
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

static void SetupConsole()
{
    DWORD outMode = 0;
    HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if(stdoutHandle == INVALID_HANDLE_VALUE) {
        exit(GetLastError());
    }

    if(!GetConsoleMode(stdoutHandle, &outMode)) {
        exit(GetLastError());
    }

    // Enable ANSI escape codes
    outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if(!SetConsoleMode(stdoutHandle, outMode)) {
        exit(GetLastError());
    }
}
#else
static void SetupConsole() {}
#endif

Log::Log(int requestedVerbo)
    : m_requestedLogLevel(requestedVerbo)
    , m_fp(nullptr)
{
    SetupConsole();
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
    string buffer = m_buffer.str();
    if(buffer.empty()) {
        return;
    }

    if(m_useStdout) {
        m_fp = stdout;
        buffer += GetColourEnd();
    }

    if(!m_fp) {
        m_fp = fopen(m_logfile.c_str(), "a+");
    }

    if(m_fp) {
        fprintf(m_fp, "%s\n", buffer.c_str());
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
        prefix << GetColour(verbosity);
        switch(verbosity) {
        case System:
            prefix << "[" << timeString << " SYS]";
            break;

        case Error:
            prefix << "[" << timeString << " ERR]";
            break;

        case Warning:
            prefix << "[" << timeString << " WRN]";
            break;

        case Dbg:
            prefix << "[" << timeString << " DBG]";
            break;

        case Developer:
            prefix << "[" << timeString << " DVL]";
            break;
        }

        prefix << " ";
        return prefix.str();
    } else {
        return "";
    }
}

const string& Log::GetColour(int verbo)
{
    if(!m_useStdout) {
        return EMPTY_STR;
    }
    switch(verbo) {
    case System:
        return CYAN;
    case Error:
        return RED;
    case Warning:
        return YELLOW;
    default:
        return WHITE;
    }
}

const string& Log::GetColourEnd()
{
    if(m_useStdout) {
        return EMPTY_STR;
    } else {
        return COLOUR_END;
    }
}
