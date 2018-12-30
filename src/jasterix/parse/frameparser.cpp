#include "frameparser.h"
#include "itemparsing.h"
#include "jasterix_logger.h"

#include <iostream>

#include <tbb/tbb.h>

using namespace std;
using namespace nlohmann;

namespace jASTERIX {

FrameParser::FrameParser(const json& framing_definition, const json& record_definition,
                         const std::map<unsigned int, nlohmann::json>& asterix_category_definitions)
    : framing_definition_(framing_definition), data_block_definition_(record_definition),
      asterix_category_definitions_(asterix_category_definitions)
{
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
        loginf << "frame header start index " << index << " size '" << size << "'";

    index += parseHeader(data, index, size, json_data, debug);

    if (debug)
        loginf << "frame header parsed, index " << index << " bytes";

    index += parseFrames(data, index, size, json_data, debug);

    if (debug)
        loginf << "frames scoped, index " << index << " bytes";
}

size_t FrameParser::parseHeader (const char* data, size_t index, size_t size, json& target, bool debug)
{
    assert (data);
    assert (size);
    assert (index < size);

    size_t parsed_bytes {0};

    for (auto& j_item : header_items_)
    {
        parsed_bytes += parseItem(j_item, data, index+parsed_bytes, size, parsed_bytes, target, target, debug);
        // HACK target and parent same
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
                                      target["frames"][frames_cnt], target, debug);
            target["frames"][frames_cnt]["cnt"] = frames_cnt;
        }
        ++frames_cnt;
    }

    return parsed_bytes;
}

void FrameParser::decodeFrames (const char* data, json& json_data, bool debug)
{
    size_t num_frames = json_data.at("frames").size();

    if (debug) // switch to single thread in debug
    {
        for (json& frame_it : json_data.at("frames"))
        {
            decodeFrame (data, json_data, frame_it, debug);
        }
    }
    else
    {
        tbb::parallel_for( size_t(0), num_frames, [&]( size_t cnt )
        {
            decodeFrame (data, json_data, json_data.at("frames").at(cnt), debug);
        });
    }

}

size_t FrameParser::decodeFrame (const char* data, nlohmann::json& json_data, nlohmann::json& json_frame, bool debug)
{
    if (json_frame.find("content") == json_frame.end())
        throw runtime_error("frame parser scoped frames does not contain correct content");

    json& frame_content = json_frame.at("content");

    size_t index {frame_content.at("index")};
    size_t length {frame_content.at("length")};
    size_t parsed_bytes {0};

    if (debug)
        loginf << "frame parser decoding frame at index " << index << " length " << length;

    std::string data_block_name = data_block_definition_.at("name");

    for (auto& r_item : data_block_definition_.at("items"))
    {
        parsed_bytes += parseItem(r_item, data, index+parsed_bytes, length, parsed_bytes,
                                  frame_content[data_block_name], frame_content, debug);
        //++record_cnt;
    }

//    {
//        "cnt": 0,
//        "content": {
//            "index": 134,
//            "length": 56,
//            "record": {
//                "category": 1,
//                "content": {
//                    "index": 137,
//                    "length": 42
//                },
//                "length": 45
//            }
//        },
//        "frame_length": 56,
//        "frame_relative_time_ms": 2117
//    }
    // check record information
    json& record = frame_content.at(data_block_name);

    if (record.find ("category") == record.end())
        throw runtime_error("frame parser record does not contain category information");

    unsigned int cat = record.at("category");

    if (record.find ("content") == record.end())
        throw runtime_error("frame parser record does not contain content information");

    json& record_content = record.at("content");

    if (record_content.find ("index") == record_content.end())
        throw runtime_error("frame parser record content does not contain index information");

    size_t record_index = record_content.at("index");

    if (record_content.find ("length") == record_content.end())
        throw runtime_error("frame parser record content does not contain length information");

    size_t record_length = record_content.at("length");

    // try to decode
    if (asterix_category_definitions_.count(cat) != 0)
    {
        // decode
        if (debug)
            loginf << "frame parser decoding record with cat " << cat << " index " << record_index
                 << " length " << record_length;

        const json& asterix_category_definition = asterix_category_definitions_.at(cat);

        std::string record_content_name = asterix_category_definition.at("name");

        parsed_bytes = parseItem(asterix_category_definition, data, record_index, record_length,
                                 parsed_bytes, record_content[record_content_name], record_content, debug);
    }
    else if (debug)
        loginf << "frame parser decoding record with cat " << cat << " index " << record_index
             << " length " << record_length << " skipped since cat definition is missing ";

    return parsed_bytes;
}

}
