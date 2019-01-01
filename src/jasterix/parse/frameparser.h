#ifndef FRAMEPARSER_H
#define FRAMEPARSER_H

#include "json.hpp"

namespace jASTERIX {

class FrameParser
{
public:
    FrameParser(const nlohmann::json& framing_definition, const nlohmann::json& record_definition,
                const std::map<unsigned int, nlohmann::json>& asterix_category_definitions);

    // return number of parsed bytes
    size_t parseHeader (const char* data, size_t index, size_t size, nlohmann::json& target, bool debug);

    size_t parseFrames (const char* data, size_t index, size_t size, nlohmann::json& target, size_t num_frames,
                     bool debug);

    size_t decodeFrames (const char* data, nlohmann::json& target, bool debug);

    bool done() const;

private:
    const nlohmann::json framing_definition_;
    const nlohmann::json data_block_definition_;
    const std::map<unsigned int, nlohmann::json> asterix_category_definitions_;

    nlohmann::json header_items_;
    nlohmann::json frame_items_;

    bool done_ {false};

    // returns number of records
    size_t decodeFrame (const char* data, nlohmann::json& json_frame, bool debug);
};

}

#endif // FRAMEPARSER_H
