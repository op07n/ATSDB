#ifndef FRAMEPARSER_H
#define FRAMEPARSER_H

#include "json.hpp"

namespace jASTERIX {

class FrameParser
{
public:
    FrameParser(const nlohmann::json& definition);

    void scopeFrames (const char* data, size_t index, size_t size, bool debug);

    nlohmann::json data() const;

private:
    const nlohmann::json definition_;

    nlohmann::json header_items_;
    nlohmann::json frame_items_;

    nlohmann::json data_;

    // return number of parsed bytes
    size_t parseHeader (const char* data, size_t index, size_t size, nlohmann::json& target, bool debug);
    size_t parseFrames (const char* data, size_t index, size_t size, nlohmann::json& target, bool debug);
};

}

#endif // FRAMEPARSER_H
