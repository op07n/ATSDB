#include "frameparser.h"
#include "itemparsing.h"

#include <iostream>

using namespace std;
using namespace nlohmann;

namespace jASTERIX {

FrameParser::FrameParser(const json& framing_definition, const json& record_definition)
    : framing_definition_(framing_definition), record_definition_(record_definition)
{
    //cout << "constructing frame parser" << endl;

    if (framing_definition_.find("name") == framing_definition_.end())
        throw runtime_error ("frame parser construction without JSON name definition");

    if (framing_definition_.find("header_items") == framing_definition_.end())
        throw runtime_error ("frame parser construction without header items");

    if (!framing_definition_.at("header_items").is_array())
        throw runtime_error ("frame parser construction with header items non-array");

    header_items_ = framing_definition_.at("header_items");

    if (framing_definition_.find("frame_items") == framing_definition_.end())
        throw runtime_error ("frame parser construction without frame items");

    if (!framing_definition_.at("frame_items").is_array())
        throw runtime_error ("frame parser construction with frame items non-array");

    frame_items_ = framing_definition_.at("frame_items");
}

void FrameParser::scopeFrames (const char* data, size_t index, size_t size, json& json_data,
                                           bool debug)
{
    assert (data);
    assert (size);

    if (debug)
        cout << "frame header start index " << index << " size '" << size << "'" << endl;

    index += parseHeader(data, index, size, json_data, debug);

    if (debug)
        cout << "frame header parsed, index " << index << " bytes" << endl;

    index += parseFrames(data, index, size, json_data, debug);

    if (debug)
        cout << "frames scoped, index " << index << " bytes" << endl;
}

size_t FrameParser::parseHeader (const char* data, size_t index, size_t size, json& target, bool debug)
{
    assert (data);
    assert (size);
    assert (index < size);

    size_t parsed_bytes {0};

    for (auto& j_item : header_items_)
    {
        parsed_bytes += parseItem(j_item, data, index+parsed_bytes, size, parsed_bytes, target, debug);
    }

    return parsed_bytes;
}

size_t FrameParser::parseFrames (const char* data, size_t index, size_t size, json& target, bool debug)
{
    assert (data);
    assert (size);
    assert (index < size);

    size_t parsed_bytes {0};
    size_t current_parsed_bytes {0};
    size_t frames_cnt {0};

    while (index+parsed_bytes < size)
    {
        current_parsed_bytes = 0;
        for (auto& j_item : frame_items_)
        {
            parsed_bytes += parseItem(j_item, data, index+parsed_bytes, size, current_parsed_bytes,
                                      target["frames"][frames_cnt], debug);
            target["frames"][frames_cnt]["cnt"] = frames_cnt;
        }
        ++frames_cnt;
    }

    return parsed_bytes;
}

void FrameParser::decodeFrames (const char* data, json& json_data, bool debug)
{
//    {
//       "cnt": 9998,
//       "content": {
//         "index": 1092039,
//         "length": 41
//       },
//       "frame_length": 41,
//       "frame_relative_time_ms": 27128
//    }

    size_t index;
    size_t length;

    size_t parsed_bytes {0};

    size_t record_cnt {0};
    size_t record_all_cnt {0};


    for (json& frame_it : json_data.at("frames"))
    {
        if (frame_it.find("content") == frame_it.end())
            throw runtime_error("frame parser scoped frames does not contain correct content");

        json& frame_content = frame_it.at("content");

        index = frame_content.at("index");
        length = frame_content.at("length");
        parsed_bytes = 0;
        record_cnt = 0;

        if (debug)
            cout << "frame parser decoding frame at index " << index << " length " << length << endl;

        for (auto& r_item : record_definition_.at("items"))
        {
            parsed_bytes += parseItem(r_item, data, index+parsed_bytes, length, parsed_bytes,
                                      frame_content["record"], debug);
            //target["frames"][frames_cnt]["cnt"] = frames_cnt;
            ++record_cnt;
            ++record_all_cnt;
        }
    }

}

}
