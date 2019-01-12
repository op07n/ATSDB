#ifndef FIXEDBITFIELDITEMPARSER_H
#define FIXEDBITFIELDITEMPARSER_H

#include "itemparser.h"
#include "json.hpp"

#include <vector>
#include <memory>

namespace jASTERIX
{

class FixedBitFieldItemParser : public ItemParser
{
public:
    FixedBitFieldItemParser (const nlohmann::json& item_definition);
    virtual ~FixedBitFieldItemParser() {}

    virtual size_t parseItem (const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                              nlohmann::json& target, bool debug) override;
protected:
    bool optional_{false};
    std::string optional_variable_name_;
    nlohmann::json optional_variable_value_;
    size_t length_; // byte length
    std::vector<std::unique_ptr<ItemParser>> items_;
};

}


#endif // FIXEDBITFIELDITEMPARSER_H
