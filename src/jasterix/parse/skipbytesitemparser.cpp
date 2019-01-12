#include "skipbytesitemparser.h"

using namespace std;
using namespace nlohmann;


namespace jASTERIX
{

SkipBytesItemParser::SkipBytesItemParser (const nlohmann::json& item_definition)
 : ItemParser (item_definition)
{
    assert (type_ == "skip_bytes");

    if (item_definition.find("length") == item_definition.end())
        throw runtime_error ("fixed bytes item '"+name_+"' parsing without length");

    length_ = item_definition.at("length");
}

size_t SkipBytesItemParser::parseItem (const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                              nlohmann::json& target, bool debug)
{
    if (debug)
        loginf << "parsing skipped bytes item '"+name_+"' index " << index << " length " << length_;

    return length_;
}

}
