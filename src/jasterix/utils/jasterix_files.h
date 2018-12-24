#ifndef JASTERIX_FILES_H
#define JASTERIX_FILES_H

#include <string>
#include <vector>

namespace jASTERIX
{

namespace Files
{

bool fileExists(const std::string& path);
bool directoryExists(const std::string& path);
std::vector<std::string> getFilesInDirectory (const std::string& path);

}
}

#endif // JASTERIX_FILES_H
