#include "CommandLineParser.hpp"

#include "dap/StringUtils.hpp"
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

CommandLineParser::CommandLineParser() {}

CommandLineParser::~CommandLineParser() {}

void CommandLineParser::PrintUsage(const char* exename, struct option long_options[])
{
    size_t i = 0;
    stringstream ss;
    ss << "Usage: " << exename;
    while(true) {
        auto entry = long_options[i];
        if(entry.name == nullptr) {
            break;
        }
        ss << " --" << entry.name << "|-" << (char)entry.val;
        switch(entry.has_arg) {
        case no_argument:
            break;
        case required_argument:
            ss << " <" << StringUtils::ToUpper(entry.name) << ">";
            break;
        case optional_argument:
            ss << " [" << StringUtils::ToUpper(entry.name) << "]";
            break;
        }
        ++i;
    }
    cerr << ss.str() << endl;
}

void CommandLineParser::Parse(int argc, char** argv)
{
    // clang-format off
    static struct option long_options[] = 
    { 
        { "verbose", no_argument, 0, 'v' }, 
        { "host", required_argument, 0, 'h' },
        { "port", required_argument, 0, 'p' }, 
        { "debugger", required_argument, 0, 'd' }, 
        { 0, 0, 0, 0 }
    };
    // clang-format on

    int c;
    while(1) {
        // getopt_long stores the option index here
        int option_index = 0;
        c = getopt_long(argc, argv, ":vh:p:d:", long_options, &option_index);

        // Detect the end of the options.
        if(c == -1)
            break;

        switch(c) {
        case 0:
            break;
        case 'h':
            m_host = optarg;
            break;
        case 'p':
            m_port = atoi(optarg);
            break;
        case 'v':
            m_verbose = true;
            break;
        case 'd':
            m_debuggerExec = optarg;
            break;
        case '?':
            // getopt_long already printed an error message
            break;
        default:
            PrintUsage(argv[0], long_options);
            exit(1);
        }
    }

    // Store any remaining command line arguments (not options)
    while(optind < argc) {
        m_arguments.push_back(argv[optind++]);
    }
}

string CommandLineParser::GetConnectionString() const { return "tcp://" + GetHost() + ":" + to_string(m_port); }
