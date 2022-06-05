#include "Log.hpp"
#include "StringUtils.hpp"
#include <chrono>
#include <sstream>
#include <sys/time.h>
namespace dap
{
int Log::m_verbosity = Log::Error;
wxString Log::m_logfile;
bool Log::m_useStdout = false;

static const wxString GREEN = "\x1b[32m";
static const wxString RED = "\x1b[31m";
static const wxString YELLOW = "\x1b[93m";
static const wxString CYAN = "\x1b[96m";
static const wxString WHITE = "\x1b[37m";
static const wxString COLOUR_END = "\x1b[0m";
static const wxString EMPTY_STR = "";

// Needed for Windows
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
// Some old MinGW/CYGWIN distributions don't define this:
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

static DWORD initMode = 0;
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
    initMode = outMode;
    // Enable ANSI escape codes
    outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if(!SetConsoleMode(stdoutHandle, outMode)) {
        exit(GetLastError());
    }
}
static void ResetConsole()
{
    HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleMode(stdoutHandle, initMode);
}
#else
static void SetupConsole() {}
static void ResetConsole() {}
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
    ResetConsole();
}

void Log::AddLogLine(const wxString& msg, int verbosity)
{
    if(msg.empty()) {
        return;
    }
    if((m_verbosity >= verbosity)) {
        wxString formattedMsg = Prefix(verbosity);
        m_buffer << formattedMsg << " " << msg;
        m_buffer << "\n";
    }
}

void Log::SetVerbosity(int level)
{
    if(level > Log::Warning) {
        LOG_SYSTEM() << Log::GetVerbosityAsString(level) << wxString("");
    }
    m_verbosity = level;
}

int Log::GetVerbosityAsNumber(const wxString& verbosity)
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

    } else if(verbosity == "Info") {
        return Log::Info;
    } else {
        return Log::Error;
    }
}

wxString Log::GetVerbosityAsString(int verbosity)
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

    case Log::Info:
        return "Info";

    default:
        return "Error";
    }
}

void Log::SetVerbosity(const wxString& verbosity) { SetVerbosity(GetVerbosityAsNumber(verbosity)); }

void Log::OpenLog(const wxString& fullpath, int verbosity)
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
    wxString buffer = m_buffer.str();
    if(buffer.empty()) {
        return;
    }

    if(m_useStdout) {
        m_fp = stdout;
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
    m_buffer = {};
}

wxString Log::Prefix(int verbosity)
{
    if(verbosity <= m_verbosity) {
        timeval tim;
        gettimeofday(&tim, NULL);
        auto start = std::chrono::system_clock::now();
        auto as_time_t = std::chrono::system_clock::to_time_t(start);
        wxString timeString = ctime(&as_time_t);
        StringUtils::Trim(timeString);

        std::stringstream prefix;
        switch(verbosity) {
        case Info:
            prefix << "[" << timeString << "] " << GetColour(verbosity) << " [ INFO ]" << GetColourEnd();
            break;

        case System:
            prefix << "[" << timeString << "] " << GetColour(verbosity) << " [ SYSTEM ]" << GetColourEnd();
            break;

        case Error:
            prefix << "[" << timeString << "] " << GetColour(verbosity) << " [ ERROR ]" << GetColourEnd();
            break;

        case Warning:
            prefix << "[" << timeString << "] " << GetColour(verbosity) << " [ WARNING ]" << GetColourEnd();
            break;

        case Dbg:
            prefix << "[" << timeString << "] " << GetColour(verbosity) << " [ DEBUG ]" << GetColourEnd();
            break;

        case Developer:
            prefix << "[" << timeString << "] " << GetColour(verbosity) << " [ TRACE ]" << GetColourEnd();
            break;
        }

        prefix << " ";
        return prefix.str();
    } else {
        return "";
    }
}

const wxString& Log::GetColour(int verbo)
{
    if(!m_useStdout) {
        return EMPTY_STR;
    }
    switch(verbo) {
    case Info:
        return GREEN;
    case System:
        return CYAN;
    case Error:
        return RED;
    case Warning:
        return YELLOW;
    case Dbg:
        return CYAN;
    default:
        return WHITE;
    }
}

const wxString& Log::GetColourEnd()
{
    if(!m_useStdout) {
        return EMPTY_STR;
    } else {
        return COLOUR_END;
    }
}
}; // namespace dap