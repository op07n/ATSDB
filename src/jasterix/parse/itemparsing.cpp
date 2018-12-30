#include "itemparsing.h"
#include "jasterix_logger.h"

#include <iostream>
#include <cassert>
#include <string>
#include <exception>

using namespace std;
using namespace nlohmann;

namespace jASTERIX
{

size_t parseItem (const nlohmann::json& item_definition, const char* data, size_t index, size_t size,
                  size_t current_parsed_bytes, json& target, json& parent, bool debug)
{
    assert (data);
    assert (size);

    if (item_definition.find("name") == item_definition.end())
        throw runtime_error ("item parsing without JSON name definition");

    std::string name = item_definition.at("name");

    if (item_definition.find("type") == item_definition.end())
        throw runtime_error ("item '"+name+"' parsing without data type definition");

    std::string type = item_definition.at("type");

    const char* current_data = &data[index];

    if (type == "fixed_bytes")
    {
        if (debug)
            loginf << "parsing fixed bytes item '" << name << "'";

        if (item_definition.find("length") == item_definition.end())
            throw runtime_error ("fixed bytes item '"+name+"' parsing without length");

        unsigned int length = item_definition.at("length");

        if (item_definition.find("data_type") == item_definition.end())
            throw runtime_error ("fixed bytes item '"+name+"' parsing without data type");

        std::string data_type = item_definition.at("data_type");

        if (data_type == "string")
        {
            std::string data_str (reinterpret_cast<char const*>(current_data), length-1); // -1 to account for end 0

            if (debug)
                loginf << "fixed bytes item '"+name+"' parsing index " << index << " length " << length
                     << " data type " << data_type << " value '" << data_str << "'";

            assert (target.find(name) == target.end());
            target[name] = data_str;

            return length;
        }
        else if (data_type == "uint")
        {
            if (length > sizeof(size_t))
                throw runtime_error ("fixed bytes item '"+name+"' length larger than "+to_string(sizeof(size_t)));

            size_t data_uint {0};

            bool reverse_bits = (item_definition.find("reverse_bits") != item_definition.end()
                    && item_definition.at("reverse_bits") == true);

            unsigned char tmp;

            if (item_definition.find("reverse_bytes") != item_definition.end()
                    && item_definition.at("reverse_bytes") == true)
            {

                for (size_t cnt = 0; cnt < length; ++cnt)
                {
                    tmp = *reinterpret_cast<const unsigned char*> (&current_data[cnt]);

                    if (reverse_bits)
                        reverseBits(tmp);

                    data_uint = (data_uint << 8) + tmp;
                }
            }
            else
            {
                for (int cnt = length-1; cnt >= 0; --cnt)
                {
                    tmp = *reinterpret_cast<const unsigned char*> (&current_data[cnt]);

                    if (reverse_bits)
                        reverseBits(tmp);

                    data_uint = (data_uint << 8) + tmp;
                }
            }

            if (debug)
                loginf << "parsing fixed bytes item '"+name+"' index " << index << " length " << length
                     << " data type " << data_type << " value '" << data_uint << "'";

            assert (target.find(name) == target.end());
            target[name] = data_uint;

            return length;
        }
        else
            throw runtime_error ("fixed bytes item '"+name+"' parsing with unknown data type '"+data_type+"'");
    }
    else if (type == "skip_bytes")
    {
        if (debug)
            loginf << "parsing skip bytes item '" << name << "'";

        if (item_definition.find("length") == item_definition.end())
            throw runtime_error ("fixed bytes item '"+name+"' parsing without length");

        unsigned int length = item_definition.at("length");

        if (debug)
            loginf << "parsing skipped bytes item '"+name+"' index " << index << " length " << length;

        return length;
    }
    else if (type == "dynamic_bytes")
    {
        if (debug)
            loginf << "parsing dynamic bytes item '" << name << "'";

        if (item_definition.find("length_variable") == item_definition.end())
            throw runtime_error ("dynamic bytes item '"+name+"' parsing without length variable");

        string length_variable_name = item_definition.at("length_variable");

        if (target.find(length_variable_name) == target.end())
            throw runtime_error ("dynamic bytes item '"+name+"' parsing without given length");

        size_t length = target.at(length_variable_name);

        if (item_definition.find("substract_previous") != item_definition.end()
                && item_definition.at("substract_previous") == true)
        {
            length -= current_parsed_bytes;
            assert (length - current_parsed_bytes >= 0);
        }

        if (debug)
            loginf << "parsing dynamic bytes item '"+name+"' index " << index << " length " << length;

        assert (target.find(name) == target.end());
        target[name]["index"] = index;
        target[name]["length"] = length;

        return length;
    }
    else if (type == "compound")
    {
        if (debug)
            loginf << "parsing compound item '" << name << "'";

        if (item_definition.find("field_specification") == item_definition.end())
            throw runtime_error ("compound item '"+name+"' parsing without field specification");

        const json& field_specification = item_definition.at("field_specification");

        if (!field_specification.is_object())
            throw runtime_error ("parsing compound item '"+name+"' field specification is not an object");

        std::string field_specification_name = field_specification.at("name");

        if (item_definition.find("items") == item_definition.end())
            throw runtime_error ("parsing compound item '"+name+"' without items");

        const json& items = item_definition.at("items");

        if (!items.is_array())
            throw runtime_error ("parsing compound item '"+name+"' field specification is not an array");

        if (debug)
            loginf << "parsing compound item '" << name << "' with " << items.size() << " items";

        size_t parsed_bytes {0};

        if (debug)
            loginf << "parsing compound item '"+name+"' field specification";

        parsed_bytes = parseItem(field_specification, data, index+parsed_bytes, size,
                                 parsed_bytes, target[field_specification_name], target, debug);

        if (debug)
            loginf << "parsing compound item '"+name+"' data items";

        std::string item_name;

        for (const json& data_item_it : items)
        {
            item_name = data_item_it.at("name");

            if (debug)
                loginf << "parsing compound item '" << name << "' data item '" << item_name << "'";

            parsed_bytes = parseItem(data_item_it, data, index+parsed_bytes, size,
                                     parsed_bytes, target[item_name], target, debug);
        }

        return parsed_bytes;
    }
    else if (type == "extendable_bits")
    {
        if (debug)
            loginf << "parsing extendable bits item '" << name << "'";

        if (item_definition.find("data_type") == item_definition.end())
            throw runtime_error ("extendable bits item '"+name+"' parsing without data type");

        std::string data_type = item_definition.at("data_type");

        if (data_type == "bitfield")
        {
            unsigned int bitmask;
            std::vector <bool> bitfield;
            bool value = true;
            size_t parsed_bytes {0};

            while (value != false) // last value is extension bit
            {
                const unsigned char current_byte =
                        *reinterpret_cast<const unsigned char*> (&current_data[parsed_bytes]);

                bitmask = 1;

                for (size_t cnt{0}; cnt < 8; ++cnt)
                {
                    value = current_byte & bitmask;
                    bitfield.push_back(value);

                    if (debug)
                        loginf << "extendable bits item '" << name << "' index " << index+parsed_bytes
                               << " current byte " << static_cast<unsigned int>(current_byte)
                               << " bit field index " << cnt << " bitmask " << bitmask << " value " << value;

                    bitmask = bitmask << 1;
                }
                ++parsed_bytes;
            }

            target = bitfield;

            if (debug)
                loginf << "extendable bits item '"+name+"'" << " index " << index
                       << " parsed " << parsed_bytes << " bytes";

            return parsed_bytes;
        }
        else
            throw runtime_error ("extentable bits item '"+name+"' parsing with unknown data type '"+data_type+"'");
    }
    else if (type == "fixed_byte_bitfield")
    {
        if (debug)
            loginf << "parsing fixed byte bitfield item '" << name << "'";

        if (item_definition.find("items") == item_definition.end())
            throw runtime_error ("parsing fixed byte bitfield item '"+name+"' without sub-items");

        const json& items = item_definition.at("items");

        if (!items.is_array())
            throw runtime_error ("parsing fixed byte bitfield item '"+name+"' sub-items specification is not an array");

        std::string subitem_name;

        for (const json& sub_item_it : items)
        {
            subitem_name = sub_item_it.at("name");

            if (debug)
                loginf << "parsing fixed byte bitfield item '" << name << "' item '" << subitem_name << "'";

            parseItem(sub_item_it, data, index, size, 0, target[subitem_name], target, debug);
        }

        return 1;
    }
    else if (type == "fixed_bits")
    {
        if (debug)
            loginf << "parsing fixed bits item '" << name << "'";

        if (item_definition.find("start_bit") == item_definition.end())
            throw runtime_error ("parsing fixed byte bitfield item '"+name+"' without start bit");

        unsigned int start_bit = item_definition.at("start_bit");

        if (item_definition.find("length") == item_definition.end())
            throw runtime_error ("parsing fixed byte bitfield item '"+name+"' without length");

        unsigned int length = item_definition.at("length");

        if (start_bit+length > 8)
            throw runtime_error ("parsing fixed byte bitfield item '"+name+"' without improper length");

        const unsigned char current_byte =
                *reinterpret_cast<const unsigned char*> (&current_data[0]);

        if (debug)
            loginf << "parsing fixed bits item '" << name << "'"
                   << " current byte " << static_cast<unsigned int>(current_byte)
                   << " with start bit " << start_bit << " length " << length;

        unsigned int bitmask {1};
        bitmask <<= start_bit;

        bool bit_set {false};
        unsigned int value {0};

        for (unsigned cnt=0; cnt < length; ++cnt)
        {
            value <<= 1;
            bit_set = current_byte & bitmask;
            value |= bit_set;
        }

        if (debug)
            loginf << "parsing fixed bits item '" << name << "' with start bit " << start_bit << " length " << length
                   << " value " << value;

        target = value;

        return 0;
    }
    else if (type == "optional_item")
    {
        if (debug)
            loginf << "parsing optional item '" << name << "'";

        if (item_definition.find("optional_bitfield_name") == item_definition.end())
            throw runtime_error ("optional item '"+name+"' parsing without bitfield name");

        string bitfield_name = item_definition.at("optional_bitfield_name");

        if (item_definition.find("optional_bitfield_index") == item_definition.end())
            throw runtime_error ("optional item '"+name+"' parsing without bitfield index");

        unsigned int bitfield_index = item_definition.at("optional_bitfield_index");

        assert (parent != nullptr);

        if (parent.find(bitfield_name) == parent.end())
            throw runtime_error ("parsing optional item '"+name+"' without defined bitfield '"+bitfield_name+"'");

        const json& bitfield = parent.at(bitfield_name);

        if (!bitfield.is_array() || bitfield_index >= bitfield.size() || !bitfield.at(bitfield_index).is_boolean())
            throw runtime_error ("parsing optional item '"+name+"' with improper bitfield '"+bitfield_name+"'");

        if (debug)
            loginf << "parsing optional item '" << name << "' bitfield length " << bitfield.size()
                   << " index " << bitfield_index;

        bool item_exists = bitfield.at(bitfield_index);

        if (item_definition.find("data_fields") == item_definition.end())
            throw runtime_error ("parsing optional item '"+name+"' without sub-items");

        const json& data_fields = item_definition.at("data_fields");

        if (!data_fields.is_array())
            throw runtime_error ("parsing optional item '"+name+"' data fields container is not an array");

        if (debug)
            loginf << "parsing optional item '" << name << "' with " << data_fields.size() << " data fields, exists "
                   << item_exists;

        size_t parsed_bytes {0};

        if (debug)
            loginf << "parsing optional item '"+name+"' sub-items";

        for (auto& df_item : data_fields)
        {
            parsed_bytes += parseItem(df_item, data, index+parsed_bytes, size, current_parsed_bytes,
                                      target, parent, debug);
        }

        if (debug)
            loginf << "parsing optional item '"+name+"' done, " << parsed_bytes << " bytes parsed";

        return parsed_bytes;
    }
    else
        throw runtime_error ("item parsing name '"+name+"' with unknown type '"+type+"'");
}

unsigned char reverseBits(unsigned char b)
{
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;

   return b;
}


}
