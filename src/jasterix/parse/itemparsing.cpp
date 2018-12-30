#include "itemparsing.h"
#include "jasterix_logger.h"

#include <iostream>
#include <cassert>
#include <string>
#include <exception>

using namespace std;

namespace jASTERIX
{

size_t parseItem (const nlohmann::json& item_definition, const char* data, size_t index, size_t size,
                  size_t current_parsed_bytes, nlohmann::json& target, bool debug)
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
                loginf << "fixed bytes item '"+name+"' index " << index << " length " << length
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
                loginf << "fixed bytes item '"+name+"' index " << index << " length " << length
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
            loginf << "skip bytes item '"+name+"' index " << index << " length " << length;

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
            loginf << "dynamic bytes item '"+name+"' index " << index << " length " << length;

        assert (target.find(name) == target.end());
        target[name]["index"] = index;
        target[name]["length"] = length;

        return length;
    }
    else
        throw runtime_error ("item parsing name '"+name+"' with unknown type '"+type+"'");
}


}
