
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

#include <iostream>
#include <cstdlib>

#include "jasterix.h"

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

namespace po = boost::program_options;

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

using namespace std;

int main (int argc, char **argv)
{
    signal(SIGSEGV, handler);   // install our handler

    std::string filename;
    std::string framing {"netto"};
    std::string definition_path;
    bool debug {false};

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("filename", po::value<std::string>(&filename), "input file name")
        ("definition_path", po::value<std::string>(&definition_path), "path to jASTERIX definition files")
        ("framing", po::value<std::string>(&framing), "input framine format, as specified in the framing definitions."
                                                      " netto is default")
        ("debug", po::bool_switch(&debug), "print debug output")
    ;

    try
    {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            std::cout << desc << "\n";
            return 1;
        }
    }
    catch (exception& e)
    {
        cerr << "jASTERIX: unable to parse command line parameters: " << endl
                  << e.what() << endl;
        return 0;
    }

    // check if basic configuration works
    try
    {
        if (debug)
            cout << "jASTERIX: startup with filename '" << filename << "' framing '" << framing
                 << "' definition_path '" << definition_path << "' debug " << debug << endl;

        jASTERIX::jASTERIX asterix (filename, definition_path, framing, debug);
        boost::posix_time::ptime  start_time = boost::posix_time::microsec_clock::local_time();

        asterix.scopeFrames();

        boost::posix_time::ptime stop_time = boost::posix_time::microsec_clock::local_time();
        boost::posix_time::time_duration diff = stop_time - start_time;

        string time_str = to_string(diff.hours())+"h "+to_string(diff.minutes())+"m "+to_string(diff.seconds())+"s";

        //if (debug)
            cout << "jASTERIX: scoped frames in " << time_str << endl;
    }
    catch (exception &ex)
    {
        cerr  << "jASTERIX: caught exception: " << ex.what() << endl;

        //assert (false);

        return -1;
    }
    catch(...)
    {
        cerr  << "jASTERIX: caught exception" << endl;

        //assert (false);

        return -1;
    }

    cout << "jASTERIX: shutdown" << endl;
    //std::cout.flush();

    return 0;
}
