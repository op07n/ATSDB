#include "jasterix_files.h"

#include <cassert>
#include <stdexcept>
#include <iostream>

#include <boost/filesystem.hpp>


namespace jASTERIX
{

namespace Files
{

bool fileExists(const std::string& path)
{
    return boost::filesystem::exists(path);
}

bool directoryExists(const std::string& path)
{
    return boost::filesystem::exists(path) && boost::filesystem::is_directory(path);
}

struct path_leaf_string
{
    std::string operator()(const boost::filesystem::directory_entry& entry) const
    {
        return entry.path().leaf().string();
    }
};

std::vector<std::string> getFilesInDirectory(const std::string& path)
{
    std::vector<std::string> tmp;

    boost::filesystem::path p(path);
    boost::filesystem::directory_iterator start(p);
    boost::filesystem::directory_iterator end;
    std::transform(start, end, std::back_inserter(tmp), path_leaf_string());

    return tmp;
}

}
}
