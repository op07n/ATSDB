#ifndef ITEMPARSING_H
#define ITEMPARSING_H

#include <cstddef>
#include <sstream>

#include "json.hpp"

namespace jASTERIX
{
    // return number of parsed bytes
size_t parseItem (const nlohmann::json& item_definition, const char* data, size_t index, size_t size,
                  size_t current_parsed_bytes, nlohmann::json& target, nlohmann::json& parent, bool debug);

bool variableHasValue (const nlohmann::json& data, const std::string& variable_name,
                       const nlohmann::json& variable_value);

unsigned char reverseBits(unsigned char b);

inline std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return std::move(elems);
}

}

#endif // ITEMPARSING_H
