#ifndef FIXEDBYTESITEMPARSER_H
#define FIXEDBYTESITEMPARSER_H

#include "itemparser.h"

namespace jASTERIX
{

class FixedBytesItemParser : public ItemParser
{
public:
    FixedBytesItemParser (const nlohmann::json& item_definition);
    virtual ~FixedBytesItemParser() {}

    virtual size_t parseItem (const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                              nlohmann::json& target, bool debug) override;

protected:
    size_t length_{0};
    std::string data_type_;
    bool reverse_bits_{false};
    bool reverse_bytes_{false};
    unsigned int negative_bit_pos_ {0};
};

}

#endif // FIXEDBYTESITEMPARSER_H
