#include "frame_parser.h"

namespace jASTERIX {

FrameParser::FrameParser(const nlohmann::json& definition)
    : definition_(definition)
{
    if (definition_.find("name") == definition_.end())
        throw std::runtime_error ("frame parser construction without JSON name definition");

    if (definition_.find("header_items") == definition_.end())
        throw std::runtime_error ("frame parser construction without header items");

    if (!definition_.at("header_items").is_array())
        throw std::runtime_error ("frame parser construction with header items non-array");

    header_items_ = definition_.at("header_items");

    if (definition_.find("frame_items") == definition_.end())
        throw std::runtime_error ("frame parser construction without frame items");

    if (!definition_.at("frame_items").is_array())
        throw std::runtime_error ("frame parser construction with frame items non-array");

    frame_items_ = definition_.at("frame_items");
}

size_t FrameParser::parseHeader (nlohmann::json& target)
{

}

size_t FrameParser::parseFrames (nlohmann::json& target)
{

}


}
