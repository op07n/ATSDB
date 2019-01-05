#ifndef EXTENDABLEITEMPARSER_H
#define EXTENDABLEITEMPARSER_H

#include "itemparser.h"

#include <vector>
#include <memory>

namespace jASTERIX
{

// parses item into array, extend into next has to signified by bool extend
class ExtendableItemParser : public ItemParser
{
public:
    ExtendableItemParser (const nlohmann::json& item_definition);
    virtual ~ExtendableItemParser() {}

    virtual size_t parseItem (const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                              nlohmann::json& target, nlohmann::json& parent, bool debug);
protected:
    std::vector<std::unique_ptr<ItemParser>> items_;
};

}


#endif // EXTENDABLEITEMPARSER_H
