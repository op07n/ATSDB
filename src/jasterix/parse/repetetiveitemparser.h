#ifndef REPETETIVEITEMPARSER_H
#define REPETETIVEITEMPARSER_H

#include "itemparser.h"

namespace jASTERIX
{

class RepetetiveItemParser : public ItemParser
{
public:
    RepetetiveItemParser (const nlohmann::json& item_definition);
    virtual ~RepetetiveItemParser() {}

    virtual size_t parseItem (const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                              nlohmann::json& target, bool debug) override;
protected:
    std::unique_ptr<ItemParser> repetition_item_;
    std::vector<std::unique_ptr<ItemParser>> items_;
};

}

#endif // REPETETIVEITEMPARSER_H
