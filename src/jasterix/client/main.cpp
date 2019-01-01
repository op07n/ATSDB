
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

#include <iostream>
#include <cstdlib>

#include "jasterix.h"
#include "jasterix_logger.h"

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/Layout.hh"
#include "log4cpp/SimpleLayout.hh"

namespace po = boost::program_options;

//void handler(int sig) {
//  void *array[10];
//  size_t size;

//  // get void*'s for all entries on the stack
//  size = backtrace(array, 10);

//  // print out all the frames to stderr
//  fprintf(stderr, "Error: signal %d:\n", sig);
//  backtrace_symbols_fd(array, size, STDERR_FILENO);
//  exit(1);
//}

using namespace std;

int main (int argc, char **argv)
{
    static_assert (sizeof(size_t) >= 8, "code requires size_t with at least 8 bytes");

    //signal(SIGSEGV, handler);   // install our handler

    // setup logging
    log4cpp::Appender *console_appender_ = new log4cpp::OstreamAppender("console", &std::cout);
    console_appender_->setLayout(new log4cpp::SimpleLayout());

    log4cpp::Category& root = log4cpp::Category::getRoot();
    root.setPriority(log4cpp::Priority::INFO);
    root.addAppender(console_appender_);

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
            loginf << desc;
            return 1;
        }
    }
    catch (exception& e)
    {
        logerr << "jASTERIX: unable to parse command line parameters: \n" << e.what();
        return -1;
    }

    // check if basic configuration works
    try
    {
        if (debug)
            loginf << "jASTERIX: startup with filename '" << filename << "' framing '" << framing
                   << "' definition_path '" << definition_path << "' debug " << debug;

        jASTERIX::jASTERIX asterix (filename, definition_path, framing, debug);
        boost::posix_time::ptime  start_time = boost::posix_time::microsec_clock::local_time();

        size_t num_frames = asterix.scopeFrames();

        boost::posix_time::time_duration diff = boost::posix_time::microsec_clock::local_time() - start_time;

        string time_str = to_string(diff.hours())+"h "+to_string(diff.minutes())+"m "+to_string(diff.seconds())+"s "+
                to_string(diff.total_milliseconds())+"ms";

        double seconds = diff.total_seconds()+diff.total_milliseconds()/1000.0;

        //if (debug)
            loginf << "jASTERIX: scoped " << num_frames << " frames in " << time_str << " "
                 << num_frames/seconds << " fr/s";

        asterix.decodeFrames();

        diff = boost::posix_time::microsec_clock::local_time() - start_time;

        time_str = to_string(diff.hours())+"h "+to_string(diff.minutes())+"m "+to_string(diff.seconds())+"s "+
                to_string(diff.total_milliseconds())+"ms";

        seconds = diff.total_seconds()+diff.total_milliseconds()/1000.0;

        //if (debug)
            loginf << "jASTERIX: decoded " << num_frames << " frames in " << time_str << " "
                 << num_frames/seconds << " fr/s";

        if (debug)
            asterix.printData();
    }
    catch (exception &ex)
    {
        logerr << "jASTERIX: caught exception: " << ex.what();

        //assert (false);

        return -1;
    }
    catch(...)
    {
        logerr << "jASTERIX: caught exception";

        //assert (false);

        return -1;
    }

    loginf << "jASTERIX: shutdown";
    //std::cout.flush();

    return 0;
}
