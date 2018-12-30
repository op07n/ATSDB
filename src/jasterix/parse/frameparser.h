#ifndef FRAMEPARSER_H
#define FRAMEPARSER_H

#include "json.hpp"

namespace jASTERIX {

class FrameParser
{
public:
    FrameParser(const nlohmann::json& framing_definition, const nlohmann::json& record_definition,
                const std::map<unsigned int, nlohmann::json>& asterix_category_definitions);

    void scopeFrames (const char* data, size_t index, size_t size, nlohmann::json& json_data, bool debug);
    void decodeFrames (const char* data, nlohmann::json& json_data, bool debug);

private:
    const nlohmann::json framing_definition_;
    const nlohmann::json data_block_definition_;
    const std::map<unsigned int, nlohmann::json> asterix_category_definitions_;

    nlohmann::json header_items_;
    nlohmann::json frame_items_;

    // return number of parsed bytes
    size_t parseHeader (const char* data, size_t index, size_t size, nlohmann::json& target, bool debug);
    size_t parseFrames (const char* data, size_t index, size_t size, nlohmann::json& target, bool debug);
};

}

#endif // FRAMEPARSER_H
