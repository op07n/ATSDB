#ifndef DYNAMICBYTESITEMPARSER_H
#define DYNAMICBYTESITEMPARSER_H

#include "itemparser.h"

namespace jASTERIX
{
// calculates the index and length based on other decoded variable
class DynamicBytesItemParser : public ItemParser
{
public:
    DynamicBytesItemParser (const nlohmann::json& item_definition);
    virtual ~DynamicBytesItemParser() {}

    virtual size_t parseItem (const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                              nlohmann::json& target, bool debug) override;
protected:
    std::string length_variable_name_;
    bool substract_previous_{false};
};

}

#endif // DYNAMICBYTESITEMPARSER_H
