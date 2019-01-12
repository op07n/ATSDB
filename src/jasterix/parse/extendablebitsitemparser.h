#ifndef EXTENDABLEBITSITEMPARSER_H
#define EXTENDABLEBITSITEMPARSER_H


#include "itemparser.h"

namespace jASTERIX
{

// parses all bits per byte into array<bool>, the last of each byte signifying the extension into next byte
class ExtendableBitsItemParser : public ItemParser
{
public:
    ExtendableBitsItemParser (const nlohmann::json& item_definition);
    virtual ~ExtendableBitsItemParser() {}

    virtual size_t parseItem (const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                              nlohmann::json& target, bool debug) override;

protected:
    std::string data_type_;
    bool reverse_bits_{false};
};

}

#endif // EXTENDABLEBITSITEMPARSER_H
