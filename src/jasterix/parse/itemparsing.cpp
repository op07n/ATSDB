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
                  size_t current_parsed_bytes, json& target, bool debug)
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

            if (item_definition.find("reverse") != item_definition.end() && item_definition.at("reverse") == true)
            {
                for (size_t cnt = 0; cnt < length; ++cnt)
                    data_uint = (data_uint << 8) + *reinterpret_cast<const unsigned char*> (&current_data[cnt]);
            }
            else
            {
                for (int cnt = length-1; cnt >= 0; --cnt)
                    data_uint = (data_uint << 8) + *reinterpret_cast<const unsigned char*> (&current_data[cnt]);
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
        if (item_definition.find("length") == item_definition.end())
            throw runtime_error ("fixed bytes item '"+name+"' parsing without length");

        unsigned int length = item_definition.at("length");

        if (debug)
            loginf << "parsing skipped bytes item '"+name+"' index " << index << " length " << length;

        return length;
    }
    else if (type == "dynamic_bytes")
    {
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
            loginf << "parsing compound item '"+name+"' with " << items.size() << " items";

        size_t parsed_bytes {0};

        if (debug)
            loginf << "parsing compound item '"+name+"' field specification";

        parsed_bytes = parseItem(field_specification, data, index+parsed_bytes, size,
                                 parsed_bytes, target[field_specification_name], debug);

        //        assert (target.find(name) == target.end());
//        target[name]["index"] = index;
//        target[name]["length"] = length;

        return parsed_bytes;
    }
    else if (type == "extendable_bits")
    {
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
                        loginf << "extendable bits item '" << name
                               << "' current byte " << static_cast<unsigned int>(current_byte)
                               << " index " << index+parsed_bytes << " bit field index " << cnt
                               << " bitmask " << bitmask << " value " << value;

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
    else
        throw runtime_error ("item parsing name '"+name+"' with unknown type '"+type+"'");
}


}
