#include "extendablebitsitemparser.h"

using namespace std;
using namespace nlohmann;

namespace jASTERIX
{

ExtendableBitsItemParser::ExtendableBitsItemParser (const nlohmann::json& item_definition)
    : ItemParser (item_definition)
{
    assert (type_ == "extendable_bits");

    if (item_definition.find("data_type") == item_definition.end())
        throw runtime_error ("extendable bits item '"+name_+"' parsing without data type");

    data_type_ = item_definition.at("data_type");

    reverse_bits_ = (item_definition.find("reverse_bits") != item_definition.end()
            && item_definition.at("reverse_bits") == true);

}

size_t ExtendableBitsItemParser::parseItem (const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                                            nlohmann::json& target, bool debug)
{
    if (debug)
        loginf << "parsing extendable bits item '" << name_ << "'";

    const char* current_data = &data[index];

    if (data_type_ == "bitfield")
    {
        unsigned int bitmask;
        std::vector <bool> bitfield;
        bool value = true;
        size_t parsed_bytes {0};

        while (value != false) // last value is extension bit
        {
            const unsigned char current_byte =
                    *reinterpret_cast<const unsigned char*> (&current_data[parsed_bytes]);

            if (reverse_bits_)
            {
                bitmask = 1;
                bitmask <<= 7;

                for (size_t cnt{0}; cnt < 8; ++cnt)
                {
                    value = current_byte & bitmask;
                    bitfield.push_back(value);

                    if (debug)
                        loginf << "extendable bits item '" << name_ << "' index " << index+parsed_bytes
                               << " current byte " << static_cast<unsigned int>(current_byte)
                               << " reverse true "
                               << " bit field index " << cnt << " bitmask " << bitmask << " value " << value;

                    bitmask >>= 1;
                }
            }
            else
            {
                bitmask = 1;

                for (size_t cnt{0}; cnt < 8; ++cnt)
                {
                    value = current_byte & bitmask;
                    bitfield.push_back(value);

                    if (debug)
                        loginf << "extendable bits item '" << name_ << "' index " << index+parsed_bytes
                               << " current byte " << static_cast<unsigned int>(current_byte)
                               << " reverse false "
                               << " bit field index " << cnt << " bitmask " << bitmask << " value " << value;

                    bitmask = bitmask << 1;
                }
            }
            ++parsed_bytes;
        }

        target.emplace(name_, bitfield);

        if (debug)
            loginf << "extendable bits item '"+name_+"'" << " index " << index
                   << " parsed " << parsed_bytes << " bytes";

        return parsed_bytes;
    }
    else
        throw runtime_error ("extentable bits item '"+name_+"' parsing with unknown data type '"+data_type_+"'");
}

}
