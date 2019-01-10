#include "dynamicbytesitemparser.h"

using namespace std;
using namespace nlohmann;

namespace jASTERIX
{

DynamicBytesItemParser::DynamicBytesItemParser (const nlohmann::json& item_definition)
 : ItemParser (item_definition)
{
    assert (type_ == "dynamic_bytes");

    if (item_definition.find("length_variable") == item_definition.end())
        throw runtime_error ("dynamic bytes item '"+name_+"' parsing without length variable");

    length_variable_name_ = item_definition.at("length_variable");

    substract_previous_ = item_definition.find("substract_previous") != item_definition.end()
            && item_definition.at("substract_previous") == true;
}

size_t DynamicBytesItemParser::parseItem (const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                              nlohmann::json& target, nlohmann::json& parent, bool debug)
{
    if (debug)
        loginf << "parsing dynamic bytes item '" << name_ << "'";

    if (debug && target.find(length_variable_name_) == target.end())
        throw runtime_error ("dynamic bytes item '"+name_+"' parsing without given length");

    size_t length = target.at(length_variable_name_);

    if (substract_previous_)
    {
        length -= current_parsed_bytes;
        assert (length - current_parsed_bytes >= 0);
    }

    if (debug)
        loginf << "parsing dynamic bytes item '"+name_+"' index " << index << " length " << length;

    assert (target.find(name_) == target.end());

    //target[name_] = { {"index", index}, {"length", length} };

    target.emplace(name_, json::object({ {"index", index}, {"length", length} }));
//    target[name_]["index"] = index;
//    target[name_]["length"] = length;

    return length;
}

}
