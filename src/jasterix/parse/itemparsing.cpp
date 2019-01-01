#include "itemparsing.h"
#include "jasterix_logger.h"
#include "base64.h"

#include <iostream>
#include <cassert>
#include <string>
#include <exception>
#include <iomanip>

using namespace std;
using namespace nlohmann;

namespace jASTERIX
{

size_t parseItem (const nlohmann::json& item_definition, const char* data, size_t index, size_t size,
                  size_t current_parsed_bytes, json& target, json& parent, bool debug)
{
    assert (data);
    assert (size);

    if (debug && item_definition.find("name") == item_definition.end())
        throw runtime_error ("item parsing without JSON name definition");

    std::string name = item_definition.at("name");

    if (debug && item_definition.find("type") == item_definition.end())
        throw runtime_error ("item '"+name+"' parsing without data type definition");

    std::string type = item_definition.at("type");

    if (type == "fixed_bytes")
    {
        return parseFixedBytesItem(name, type, item_definition, data, index, size, current_parsed_bytes, target, parent,
                                   debug);
    }
    else if (type == "skip_bytes")
    {
        if (debug)
            loginf << "parsing skip bytes item '" << name << "'";

        if (debug && item_definition.find("length") == item_definition.end())
            throw runtime_error ("fixed bytes item '"+name+"' parsing without length");

        unsigned int length = item_definition.at("length");

        if (debug)
            loginf << "parsing skipped bytes item '"+name+"' index " << index << " length " << length;

        return length;
    }
    else if (type == "dynamic_bytes")
    {
        return parseDynamicBytesItem(name, type, item_definition, data, index, size, current_parsed_bytes, target,
                                     parent, debug);
    }
    else if (type == "compound")
    {
        return parseCompoundItem(name, type, item_definition, data, index, size, current_parsed_bytes, target,
                                 parent, debug);
    }
    else if (type == "extendable_bits")
    {
        return parseExtendtableBitsItem(name, type, item_definition, data, index, size, current_parsed_bytes, target,
                                        parent, debug);
    }
    else if (type == "extendable")
    {
        return parseExtendtableItem(name, type, item_definition, data, index, size, current_parsed_bytes, target,
                                        parent, debug);
    }
    else if (type == "fixed_bitfield")
    {
        return parseFixedBitfieldItem(name, type, item_definition, data, index, size, current_parsed_bytes, target,
                                      parent, debug);
    }
    else if (type == "fixed_bits")
    {
        return parseFixedBitsItem(name, type, item_definition, data, index, size, current_parsed_bytes, target,
                                  parent, debug);
    }
    else if (type == "optional_item")
    {
        return parseOptionalItem(name, type, item_definition, data, index, size, current_parsed_bytes, target,
                                      parent, debug);
    }
    else if (type == "repetitive")
    {
        return parseRepetitiveItem(name, type, item_definition, data, index, size, current_parsed_bytes, target,
                                      parent, debug);
    }
    else
        throw runtime_error ("item parsing name '"+name+"' with unknown type '"+type+"'");
}

size_t parseFixedBytesItem (const std::string& name, const std::string& type, const nlohmann::json& item_definition,
                            const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                            nlohmann::json& target, nlohmann::json& parent, bool debug)
{
    assert (type == "fixed_bytes");

    const char* current_data = &data[index];

    if (debug)
        loginf << "parsing fixed bytes item '" << name << "'";

    if (debug && item_definition.find("length") == item_definition.end())
        throw runtime_error ("fixed bytes item '"+name+"' parsing without length");

    unsigned int length = item_definition.at("length");

    if (debug && item_definition.find("data_type") == item_definition.end())
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
            for (int cnt = length-1; cnt >= 0; --cnt)
            {
                tmp = *reinterpret_cast<const unsigned char*> (&current_data[cnt]);

                if (debug)
                    loginf << "fixed bytes item '"+name+"' cnt " << cnt << " byte "
                           << std::hex << static_cast<unsigned int> (tmp) << " reverse bytes false bits "
                           << reverse_bits << " data " << data_uint;

                if (reverse_bits)
                    reverseBits(tmp);

                data_uint = (data_uint << 8) + tmp;
            }
        }
        else
        {
            for (size_t cnt = 0; cnt < length; ++cnt)
            {
                tmp = *reinterpret_cast<const unsigned char*> (&current_data[cnt]);

                if (debug)
                    loginf << "fixed bytes item '"+name+"' cnt " << cnt << " byte "
                           << std::hex << static_cast<unsigned int> (tmp) << " reverse bytes true bits "
                           << reverse_bits << " data " << data_uint;

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
    else if (data_type == "int")
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
            for (int cnt = length-1; cnt >= 0; --cnt)
            {
                tmp = *reinterpret_cast<const unsigned char*> (&current_data[cnt]);

                if (debug)
                    loginf << "fixed bytes item '"+name+"' cnt " << cnt << " byte "
                           << std::hex << static_cast<unsigned int> (tmp) << " reverse bytes false bits "
                           << reverse_bits << " data " << data_uint;

                if (reverse_bits)
                    reverseBits(tmp);

                data_uint = (data_uint << 8) + tmp;
            }
        }
        else
        {
            for (size_t cnt = 0; cnt < length; ++cnt)
            {
                tmp = *reinterpret_cast<const unsigned char*> (&current_data[cnt]);

                if (debug)
                    loginf << "fixed bytes item '"+name+"' cnt " << cnt << " byte "
                           << std::hex << static_cast<unsigned int> (tmp) << " reverse bytes true bits "
                           << reverse_bits << " data " << data_uint;

                if (reverse_bits)
                    reverseBits(tmp);

                data_uint = (data_uint << 8) + tmp;
            }
        }

        unsigned int negative_bit_pos = length*8-1;
        const int negative = (data_uint & (1 << negative_bit_pos)) != 0;
        int data_int;

        if (negative)
            data_int = data_uint | ~((1 << negative_bit_pos) - 1);
        else
            data_int = data_uint;

        if (debug)
            loginf << "parsing fixed bytes item '"+name+"' index " << index << " length " << length
                   << " data type " << data_type << " value '" << data_int << "'";

        assert (target.find(name) == target.end());
        target[name] = data_int;

        return length;
    }
    if (data_type == "bin")
    {
        std::string data_str = base64_encode(reinterpret_cast<const unsigned char*>(current_data), length);

        if (debug)
        {
            stringstream ss;
            unsigned char tmp;
            for (unsigned int cnt=0; cnt < length; ++cnt)
            {
                tmp = current_data[cnt];
                ss << std::setfill('0') << std::setw(2) << std::hex << (int)tmp;
            }

            loginf << "fixed bytes item '"+name+"' parsing index " << index << " length " << length
                   << " data type " << data_type << " value '" << data_str << "'"
                   << " value hex '" << ss.str() << "'";
        }

        assert (target.find(name) == target.end());
        target[name] = data_str;

        return length;
    }
    else
        throw runtime_error ("fixed bytes item '"+name+"' parsing with unknown data type '"+data_type+"'");
}

size_t parseDynamicBytesItem (const std::string& name, const std::string& type, const nlohmann::json& item_definition,
                            const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                            nlohmann::json& target, nlohmann::json& parent, bool debug)
{
    assert (type == "dynamic_bytes");

    if (debug)
        loginf << "parsing dynamic bytes item '" << name << "'";

    if (debug && item_definition.find("length_variable") == item_definition.end())
        throw runtime_error ("dynamic bytes item '"+name+"' parsing without length variable");

    string length_variable_name = item_definition.at("length_variable");

    if (debug && target.find(length_variable_name) == target.end())
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

size_t parseCompoundItem (const std::string& name, const std::string& type, const nlohmann::json& item_definition,
                            const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                            nlohmann::json& target, nlohmann::json& parent, bool debug)
{
    assert (type == "compound");

    if (debug)
        loginf << "parsing compound item '" << name << "'";

    if (debug && item_definition.find("field_specification") == item_definition.end())
        throw runtime_error ("compound item '"+name+"' parsing without field specification");

    const json& field_specification = item_definition.at("field_specification");

    if (debug && !field_specification.is_object())
        throw runtime_error ("parsing compound item '"+name+"' field specification is not an object");

    std::string field_specification_name = field_specification.at("name");

    if (debug && item_definition.find("items") == item_definition.end())
        throw runtime_error ("parsing compound item '"+name+"' without items");

    const json& items = item_definition.at("items");

    if (debug && !items.is_array())
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
            loginf << "parsing compound item '" << name << "' data item '" << item_name << "' index "
                   << index+parsed_bytes;

        parsed_bytes += parseItem(data_item_it, data, index+parsed_bytes, size,
                                  parsed_bytes, target[item_name], target, debug);
    }

    return parsed_bytes;
}

size_t parseExtendtableBitsItem (const std::string& name, const std::string& type, const nlohmann::json& item_definition,
                            const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                            nlohmann::json& target, nlohmann::json& parent, bool debug)
{
    if (debug)
        loginf << "parsing extendable bits item '" << name << "'";

    if (debug && item_definition.find("data_type") == item_definition.end())
        throw runtime_error ("extendable bits item '"+name+"' parsing without data type");

    bool reverse_bits = (item_definition.find("reverse_bits") != item_definition.end()
            && item_definition.at("reverse_bits") == true);

    std::string data_type = item_definition.at("data_type");

    const char* current_data = &data[index];

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

            if (reverse_bits)
            {
                bitmask = 1;
                bitmask <<= 7;

                for (size_t cnt{0}; cnt < 8; ++cnt)
                {
                    value = current_byte & bitmask;
                    bitfield.push_back(value);

                    if (debug)
                        loginf << "extendable bits item '" << name << "' index " << index+parsed_bytes
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
                        loginf << "extendable bits item '" << name << "' index " << index+parsed_bytes
                               << " current byte " << static_cast<unsigned int>(current_byte)
                               << " reverse false "
                               << " bit field index " << cnt << " bitmask " << bitmask << " value " << value;

                    bitmask = bitmask << 1;
                }
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

size_t parseExtendtableItem (const std::string& name, const std::string& type,
                             const nlohmann::json& item_definition,const char* data, size_t index, size_t size,
                             size_t current_parsed_bytes, nlohmann::json& target, nlohmann::json& parent,
                             bool debug)
{
    assert (type == "extendable");

    if (debug)
        loginf << "parsing extendable item '" << name << "'";

    if (debug && item_definition.find("items") == item_definition.end())
        throw runtime_error ("parsing extendable item '"+name+"' without items");

    const json& items = item_definition.at("items");

    if (debug && !items.is_array())
        throw runtime_error ("parsing extendable item '"+name+"' items specification is not an array");

    if (debug)
        loginf << "parsing extendable item '" << name << "' with " << items.size() << " items";

    size_t parsed_bytes {0};

    if (debug)
        loginf << "parsing extendable item '"+name+"' items";

    std::string item_name;

    unsigned int extend = 1;
    unsigned int cnt = 0;

    while (extend)
    {
        for (const json& data_item_it : items)
        {
            item_name = data_item_it.at("name");

            if (debug)
                loginf << "parsing extendable item '" << name << "' data item '" << item_name << "' index "
                       << index+parsed_bytes << " cnt " << cnt;

            parsed_bytes += parseItem(data_item_it, data, index+parsed_bytes, size,
                                      parsed_bytes, target["data"][cnt], target, debug);

            if (debug && target.at("data").at(cnt).find("extend") == target.at("data").at(cnt).end())
                throw runtime_error ("parsing extendable item '"+name+"' without extend information");

            extend = target.at("data").at(cnt).at("extend");

            ++cnt;
        }
    }

    return parsed_bytes;
}

size_t parseFixedBitfieldItem (const std::string& name, const std::string& type,
                               const nlohmann::json& item_definition,const char* data, size_t index, size_t size,
                               size_t current_parsed_bytes, nlohmann::json& target, nlohmann::json& parent,
                               bool debug)
{
    if (debug)
        loginf << "parsing fixed bitfield item '" << name << "'";

    if (item_definition.find("optional") != item_definition.end() && item_definition.at("optional") == true)
    {
        if (item_definition.find("optional_variable_name") == item_definition.end())
            throw runtime_error ("parsing fixed bitfield item '"+name+"' optional but no variable given");

        std::string optional_variable_name = item_definition.at("optional_variable_name");

        if (debug && item_definition.find("optional_variable_value") == item_definition.end())
            throw runtime_error ("parsing fixed bitfield item '"+name
                                 +"' optional but no variable value given");

        const json& optional_variable_value = item_definition.at("optional_variable_value");

        if (!variableHasValue(parent, optional_variable_name, optional_variable_value))
        {
            if (debug) //  in '" << parent.dump(4) << "'"
                loginf << "parsing fixed bitfield item '" << name << "' skipped since variable '"
                       << optional_variable_name << "' not set";

            return 0; // no parse
        }
    }

    if (debug && item_definition.find("length") == item_definition.end())
        throw runtime_error ("parsing fixed bitfield item '"+name+"' without length");

    unsigned int length = item_definition.at("length");

    if (length > 8)
        throw runtime_error ("parsing fixed bitfield item '"+name+"' with too big length");

    if (debug && item_definition.find("items") == item_definition.end())
        throw runtime_error ("parsing fixed bitfield item '"+name+"' without sub-items");

    const json& items = item_definition.at("items");

    if (debug && !items.is_array())
        throw runtime_error ("parsing fixed bitfield item '"+name+"' sub-items specification is not an array");

    std::string subitem_name;

    for (const json& sub_item_it : items)
    {
        subitem_name = sub_item_it.at("name");

        if (debug)
            loginf << "parsing fixed bitfield item '" << name << "' item '" << subitem_name << "'";

        parseItem(sub_item_it, data, index, length, 0, target[subitem_name], target, debug);
    }

    return length;
}

size_t parseFixedBitsItem (const std::string& name, const std::string& type, const nlohmann::json& item_definition,
                           const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                           nlohmann::json& target, nlohmann::json& parent, bool debug)
{
    if (debug)
        loginf << "parsing fixed bits item '" << name << "'";

    if (debug && item_definition.find("start_bit") == item_definition.end())
        throw runtime_error ("parsing fixed byte bitfield item '"+name+"' without start bit");

    unsigned int start_bit = item_definition.at("start_bit");

    if (debug && item_definition.find("bit_length") == item_definition.end())
        throw runtime_error ("parsing fixed byte bitfield item '"+name+"' without bit length");

    unsigned int bit_length = item_definition.at("bit_length");
    unsigned int byte_length = size;

    size_t tmp_data{0};

    const char* current_data = &data[index];

    if (byte_length == 1)
        tmp_data = *reinterpret_cast<const unsigned char*> (&current_data[0]);
    else
    {
        unsigned char tmp;
        for (size_t cnt = 0; cnt < byte_length; ++cnt)
        {
            tmp = *reinterpret_cast<const unsigned char*> (&current_data[cnt]);

            if (debug)
                loginf << "fixed byte bitfield item '"+name+"' cnt " << cnt << " byte "
                       << std::hex << static_cast<unsigned int> (tmp) << " data " << tmp_data;

            tmp_data = (tmp_data << 8) + tmp;
        }
    }

    if (debug)
        loginf << "parsing fixed bits item '" << name << "'"
               << " byte length " << byte_length
               << " current data '" << hex << tmp_data << "'"
               << " with start bit " << start_bit << " length " << bit_length;

    size_t bitmask {1};
    bitmask <<= start_bit+bit_length-1;

    bool bit_set {false};
    size_t value {0};

    for (unsigned cnt=0; cnt < bit_length; ++cnt)
    {
        value <<= 1;
        bit_set = tmp_data & bitmask;
        value |= bit_set;

        if (debug)
            loginf << "parsing fixed bits item '" << name << "' with bit " << cnt
                   << " bitmask " << bitmask << " set " << bit_set << " value " << value;

        bitmask >>= 1;
    }

    if (debug)
        loginf << "parsing fixed bits item '" << name << "' with start bit " << start_bit
               << " length " << bit_length << " value " << value;

    target = value;

    return 0;
}

size_t parseOptionalItem (const std::string& name, const std::string& type, const nlohmann::json& item_definition,
                           const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                           nlohmann::json& target, nlohmann::json& parent, bool debug)
{
    if (debug)
        loginf << "parsing optional item '" << name << "'";

    if (debug && item_definition.find("optional_bitfield_name") == item_definition.end())
        throw runtime_error ("optional item '"+name+"' parsing without bitfield name");

    string bitfield_name = item_definition.at("optional_bitfield_name");

    if (debug && item_definition.find("optional_bitfield_index") == item_definition.end())
        throw runtime_error ("optional item '"+name+"' parsing without bitfield index");

    unsigned int bitfield_index = item_definition.at("optional_bitfield_index");

    assert (parent != nullptr);

    if (debug && parent.find(bitfield_name) == parent.end())
        throw runtime_error ("parsing optional item '"+name+"' without defined bitfield '"+bitfield_name+"'");

    const json& bitfield = parent.at(bitfield_name);

    if (debug && !bitfield.is_array())
        throw runtime_error ("parsing optional item '"+name+"' with non-array bitfield '"+bitfield_name+"'");

    if (bitfield_index >= bitfield.size())
    {
        if (debug)
            loginf << "parsing optional item '" << name << "' bitfield length " << bitfield.size()
                   << " index " << bitfield_index << " out of fspec size";
        return 0;
    }

    if (debug && !bitfield.at(bitfield_index).is_boolean())
        throw runtime_error ("parsing optional item '"+name+"' with non-boolean bitfield '"+bitfield_name+"' value");

    if (debug)
        loginf << "parsing optional item '" << name << "' bitfield length " << bitfield.size()
               << " index " << bitfield_index;

    bool item_exists = bitfield.at(bitfield_index);

    if (debug && item_definition.find("data_fields") == item_definition.end())
        throw runtime_error ("parsing optional item '"+name+"' without sub-items");

    const json& data_fields = item_definition.at("data_fields");

    if (debug && !data_fields.is_array())
        throw runtime_error ("parsing optional item '"+name+"' data fields container is not an array");

    if (debug)
        loginf << "parsing optional item '" << name << "' with " << data_fields.size() << " data fields, exists "
               << item_exists;

    if (!item_exists)
        return 0;

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

size_t parseRepetitiveItem (const std::string& name, const std::string& type, const nlohmann::json& item_definition,
                           const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                           nlohmann::json& target, nlohmann::json& parent, bool debug)
{
    assert (type == "repetitive");

    if (debug)
        loginf << "parsing repetitive item '" << name << "'";

    if (debug && item_definition.find("repetition_item") == item_definition.end())
        throw runtime_error ("repetitive item '"+name+"' parsing without repetition item specification");

    const json& repetition_item = item_definition.at("repetition_item");

    if (debug && !repetition_item.is_object())
        throw runtime_error ("parsing repetitive item '"+name+"' repetition item specification is not an object");

    if (debug && repetition_item.at("name") != "rep")
        throw runtime_error ("parsing repetitive item '"+name+"' repetition item specification has to be named 'rep'");

    if (debug && item_definition.find("items") == item_definition.end())
        throw runtime_error ("parsing repetitive item '"+name+"' without items");

    const json& items = item_definition.at("items");

    if (debug && !items.is_array())
        throw runtime_error ("parsing repetitive item '"+name+"' items specification is not an array");

    if (debug)
        loginf << "parsing repetitive item '" << name << "' with " << items.size() << " items";

    size_t parsed_bytes {0};

    if (debug)
        loginf << "parsing repetitive item '"+name+"' repetition item";

    parsed_bytes = parseItem(repetition_item, data, index+parsed_bytes, size,
                             parsed_bytes, target, target, debug);

    unsigned int rep = target.at("rep");

    if (debug)
        loginf << "parsing repetitive item '"+name+"' items " << rep << " times";

    std::string item_name;

    for (unsigned int cnt=0; cnt < rep; ++cnt)
    {
        for (const json& data_item_it : items)
        {
            item_name = data_item_it.at("name");

            if (debug)
                loginf << "parsing repetitive item '" << name << "' data item '" << item_name << "' index "
                       << index+parsed_bytes << " cnt " << cnt;

            parsed_bytes += parseItem(data_item_it, data, index+parsed_bytes, size,
                                      parsed_bytes, target["data"][cnt], target, debug);
        }
    }

    return parsed_bytes;
}

bool variableHasValue (const nlohmann::json& data, const std::string& variable_name,
                       const nlohmann::json& variable_value)
{
    const nlohmann::json* val_ptr = &data;
    std::vector <std::string> sub_keys = split(variable_name, '.');
    for (const std::string& sub_key : sub_keys)
    {
        if (val_ptr->find (sub_key) != val_ptr->end())
        {
            if (sub_key == sub_keys.back()) // last found
            {
                val_ptr = &val_ptr->at(sub_key);
                break;
            }

            if (val_ptr->at(sub_key).is_object()) // not last, step in
                val_ptr = &val_ptr->at(sub_key);
            else // not last key, and not object
            {
                val_ptr = nullptr;
                break;
            }
        }
        else // not found
        {
            val_ptr = nullptr;
            break;
        }
    }

    if (val_ptr == nullptr || *val_ptr == nullptr) // not found
        return false;
    else
        return *val_ptr == variable_value;
}

}
