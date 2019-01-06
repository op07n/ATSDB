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

    if (start_bit_+bit_length_ > byte_length*8)
        throw runtime_error ("fixed byte bitfield item '"+name_+"' wrong length "+to_string(byte_length*8)
                             +" for bitsize "+to_string(start_bit_+bit_length_));

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

    if (item_definition.find("data_type") != item_definition.end())
        data_type_ = item_definition.at("data_type");

    negative_bit_pos_ = start_bit_+bit_length_;
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

        if (data_type_ == "uint")
            target = tmp1;
        else if (data_type_ == "int")
        {
            int data_int;

            if ((tmp1 & (1 << negative_bit_pos_)) != 0)
                data_int = tmp1 | ~((1 << negative_bit_pos_) - 1);
            else
                data_int = tmp1;

            target = data_int;
        }
        else
            throw runtime_error ("fixed bits item '"+name_+"' parsing with unknown data type '"+data_type_+"'");
    }
    else if (byte_length_ <= 4)
    {
        unsigned int tmp4 = *reinterpret_cast<const unsigned int*> (&data[index]);
        tmp4 &= bitmask4;
        tmp4 >>= start_bit_;

        if (debug)
            loginf << "parsing fixed bits item '" << name_ << "' with start bit " << start_bit_
                   << " length " << bit_length_ << " value " << (size_t) tmp4;

        if (data_type_ == "uint")
            target = tmp4;
        else if (data_type_ == "int")
        {
            int data_int;

            if ((tmp4 & (1 << negative_bit_pos_)) != 0)
                data_int = tmp4 | ~((1 << negative_bit_pos_) - 1);
            else
                data_int = tmp4;

            target = data_int;
        }
        else
            throw runtime_error ("fixed bits item '"+name_+"' parsing with unknown data type '"+data_type_+"'");

    }
    else if (byte_length_ <= 8)
    {
        size_t tmp8 = *reinterpret_cast<const size_t*> (&data[index]);
        tmp8 &= bitmask8;
        tmp8 >>= start_bit_;

        if (debug)
            loginf << "parsing fixed bits item '" << name_ << "' with start bit " << start_bit_
                   << " length " << bit_length_ << " value " << (size_t) tmp8;

        if (data_type_ == "uint")
            target = tmp8;
        else if (data_type_ == "int")
        {
            long int data_int;

            if ((tmp8 & (1 << negative_bit_pos_)) != 0)
                data_int = tmp8 | ~((1 << negative_bit_pos_) - 1);
            else
                data_int = tmp8;

            target = data_int;
        }
        else
            throw runtime_error ("fixed bits item '"+name_+"' parsing with unknown data type '"+data_type_+"'");


    }


    return 0;
}

}
