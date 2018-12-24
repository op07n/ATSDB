#ifndef FRAME_PARSER_H
#define FRAME_PARSER_H

#include "json.hpp"

namespace jASTERIX {

class FrameParser
{
public:
    FrameParser(const nlohmann::json& definition);

    size_t parseHeader (nlohmann::json& target);
    size_t parseFrames (nlohmann::json& target);

private:
    const nlohmann::json definition_;

    nlohmann::json header_items_;
    nlohmann::json frame_items_;
};

}

#endif // FRAME_PARSER_H
