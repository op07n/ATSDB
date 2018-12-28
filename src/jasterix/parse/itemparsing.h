#ifndef ITEMPARSING_H
#define ITEMPARSING_H

#include <cstddef>

#include "json.hpp"

namespace jASTERIX
{
    // return number of parsed bytes
size_t parseItem (const nlohmann::json& item_definition, const char* data, size_t index, size_t size,
                  size_t current_parsed_bytes, nlohmann::json& target, bool debug);

}

#endif // ITEMPARSING_H
