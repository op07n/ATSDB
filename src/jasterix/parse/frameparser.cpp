#include "frameparser.h"
#include "itemparsing.h"

#include <iostream>

using namespace std;

namespace jASTERIX {

FrameParser::FrameParser(const nlohmann::json& definition)
    : definition_(definition)
{
    //cout << "constructing frame parser" << endl;

    if (definition_.find("name") == definition_.end())
        throw runtime_error ("frame parser construction without JSON name definition");

    if (definition_.find("header_items") == definition_.end())
        throw runtime_error ("frame parser construction without header items");

    if (!definition_.at("header_items").is_array())
        throw runtime_error ("frame parser construction with header items non-array");

    header_items_ = definition_.at("header_items");

    if (definition_.find("frame_items") == definition_.end())
        throw runtime_error ("frame parser construction without frame items");

    if (!definition_.at("frame_items").is_array())
        throw runtime_error ("frame parser construction with frame items non-array");

    frame_items_ = definition_.at("frame_items");
}

void FrameParser::scopeFrames (const char* data, size_t index, size_t size, bool debug)
{
    assert (data);
    assert (data_ == nullptr);
    assert (size);

    if (debug)
        cout << "frame header start index " << index << " size '" << size << "'" << endl;

    index += parseHeader(data, index, size, data_, debug);

    if (debug)
        cout << "frame header parsed, index " << index << " bytes, JSON '" << data_.dump(4) << "'" << endl;

    index += parseFrames(data, index, size, data_, debug);

    //if (debug)
        cout << "frames scoped, index " << index << " bytes, JSON '" << data_.dump(4) << "'" << endl;
}

nlohmann::json FrameParser::data() const
{
    return data_;
}

size_t FrameParser::parseHeader (const char* data, size_t index, size_t size, nlohmann::json& target, bool debug)
{
    assert (data);
    assert (size);
    assert (index < size);

    size_t parsed_bytes {0};

    for (auto& j_item : header_items_)
    {
        parsed_bytes += parseItem(j_item, data, index+parsed_bytes, size, target, debug);
    }

    return parsed_bytes;
}

size_t FrameParser::parseFrames (const char* data, size_t index, size_t size, nlohmann::json& target, bool debug)
{
    assert (data);
    assert (size);
    assert (index < size);

    size_t parsed_bytes {0};
    size_t frames_cnt {0};

    while (index+parsed_bytes < size)
    {
        for (auto& j_item : frame_items_)
        {
            parsed_bytes += parseItem(j_item, data, index+parsed_bytes, size, target["frames"][frames_cnt], debug);
        }
        ++frames_cnt;
    }

    return parsed_bytes;
}


}
