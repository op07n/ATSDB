#ifndef FIXEDBITSITEMPARSER_H
#define FIXEDBITSITEMPARSER_H

#include "itemparser.h"

namespace jASTERIX
{

class FixedBitsItemParser : public ItemParser
{
public:
    FixedBitsItemParser (const nlohmann::json& item_definition, unsigned int byte_length);
    virtual ~FixedBitsItemParser() {}

    virtual size_t parseItem (const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                              nlohmann::json& target, nlohmann::json& parent, bool debug);

protected:
    unsigned int byte_length_{0};
    unsigned int start_bit_{0};
    unsigned int bit_length_{0};
    std::string data_type_ {"uint"};
    unsigned int negative_bit_pos_{0};

    unsigned int num_digits_{0};
    unsigned int digit_bit_length_{0};

    unsigned char bitmask1;
    unsigned int bitmask4;
    size_t bitmask8;

    std::vector<unsigned char> digits_bitmasks1;
    std::vector<unsigned int> digits_bitmasks4;
    std::vector<size_t> digits_bitmasks8;
};

}

#endif // FIXEDBITSITEMPARSER_H
