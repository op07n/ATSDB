#ifndef OPTIONALITEMPARSER_H
#define OPTIONALITEMPARSER_H

#include "itemparser.h"

namespace jASTERIX
{

class OptionalItemParser : public ItemParser
{
public:
    OptionalItemParser (const nlohmann::json& item_definition);
    virtual ~OptionalItemParser() {}

    virtual size_t parseItem (const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                              nlohmann::json& target, nlohmann::json& parent, bool debug);
protected:
    std::string bitfield_name_;
    unsigned int bitfield_index_{0};
    std::vector<std::unique_ptr<ItemParser>> data_fields_;
};

}

#endif // OPTIONALITEMPARSER_H
