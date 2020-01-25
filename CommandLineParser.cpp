#include "CommandLineParser.hpp"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

CommandLineParser::CommandLineParser() {}

CommandLineParser::~CommandLineParser() {}

void CommandLineParser::Parse(int argc, char** argv)
{
    // clang-format off
    static struct option long_options[] = 
    { 
        { "verbose", no_argument, 0, 'v' }, 
        { "host", required_argument, 0, 'h' },
        { "port", required_argument, 0, 'p' }, 
        { "gdb", required_argument, 0, 'g' }, 
        { 0, 0, 0, 0 }
    };
    // clang-format on

    int c;
    while(1) {
        // getopt_long stores the option index here
        int option_index = 0;
        c = getopt_long(argc, argv, ":vh:p:g:", long_options, &option_index);

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
        case 'g':
            m_gdb = optarg;
            break;
        case '?':
            // getopt_long already printed an error message
            break;
        default:
            break;
        }
    }

    // Store any remaining command line arguments (not options)
    while(optind < argc) {
        m_arguments.push_back(argv[optind++]);
    }
}
