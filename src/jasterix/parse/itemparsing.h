#ifndef ITEMPARSING_H
#define ITEMPARSING_H

#include <cstddef>

#include "json.hpp"

namespace jASTERIX
{
    // return number of parsed bytes
size_t parseItem (const nlohmann::json& item_definition, const char* data, size_t index, size_t size,
                  nlohmann::json& target, bool debug);

}

#endif // ITEMPARSING_H
