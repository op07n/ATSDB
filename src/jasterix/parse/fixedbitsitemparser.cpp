#include "fixedbitsitemparser.h"

using namespace std;
using namespace nlohmann;

namespace jASTERIX
{

FixedBitsItemParser::FixedBitsItemParser (const nlohmann::json& item_definition, unsigned int byte_length)
 : ItemParser (item_definition), byte_length_(byte_length)
{
    assert (type_ == "fixed_bits");

    if (item_definition.find("start_bit") == item_definition.end())
        throw runtime_error ("fixed byte bitfield item '"+name_+"' without start bit");

    start_bit_ = item_definition.at("start_bit");

    if (item_definition.find("bit_length") == item_definition.end())
        throw runtime_error ("fixed byte bitfield item '"+name_+"' without bit length");

    bit_length_ = item_definition.at("bit_length");

    if (byte_length_ == 1)
    {
        bitmask1 = 1;
        for (unsigned cnt=0; cnt < bit_length_; ++cnt)
        {
            bitmask1 <<= 1;
            bitmask1 += 1;
        }
        bitmask1 <<= start_bit_;
    }
    else if (byte_length_ <= 4)
    {
        bitmask4 = 1;
        for (unsigned cnt=0; cnt < bit_length_; ++cnt)
        {
            bitmask4 <<= 1;
            bitmask4 += 1;
        }
        bitmask4 <<= start_bit_;
    }
    else if (byte_length_ <= 8)
    {
        bitmask8 = 1;
        for (unsigned cnt=0; cnt < bit_length_; ++cnt)
        {
            bitmask8 <<= 1;
            bitmask8 += 1;
        }
        bitmask8 <<= start_bit_;
    }
    else
        throw runtime_error ("fixed byte bitfield item '"+name_+"' with length"+to_string(byte_length_));
}

size_t FixedBitsItemParser::parseItem (const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                              nlohmann::json& target, nlohmann::json& parent, bool debug)
{
    if (debug)
        loginf << "parsing fixed bits item '" << name_ << "'";

    if (byte_length_ == 1)
    {
        unsigned char tmp1 = *reinterpret_cast<const unsigned char*> (&data[index]);
        tmp1 &= bitmask1;
        tmp1 >>= start_bit_;

        if (debug)
            loginf << "parsing fixed bits item '" << name_ << "' with start bit " << start_bit_
                   << " length " << bit_length_ << " value " << (size_t) tmp1;

        target = tmp1;
    }
    else if (byte_length_ <= 4)
    {
        unsigned int tmp4 = *reinterpret_cast<const unsigned int*> (&data[index]);
        tmp4 &= bitmask4;
        tmp4 >>= start_bit_;

        if (debug)
            loginf << "parsing fixed bits item '" << name_ << "' with start bit " << start_bit_
                   << " length " << bit_length_ << " value " << (size_t) tmp4;

        target = tmp4;
    }
    else if (byte_length_ <= 8)
    {
        size_t tmp8 = *reinterpret_cast<const size_t*> (&data[index]);
        tmp8 &= bitmask8;
        tmp8 >>= start_bit_;

        if (debug)
            loginf << "parsing fixed bits item '" << name_ << "' with start bit " << start_bit_
                   << " length " << bit_length_ << " value " << (size_t) tmp8;

        target = tmp8;
    }

    return 0;
}

}
