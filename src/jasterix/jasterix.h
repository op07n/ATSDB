#ifndef JASTERIX_H
#define JASTERIX_H

#include <string>
#include <map>

#include "json.hpp"
#include "frame_parser.h"

namespace jASTERIX {

class jASTERIX
{
public:
    jASTERIX(const std::string& filename, const std::string& definition_path, const std::string& framing_str);

private:
    std::string filename_;
    std::string definition_path_;
    std::string framing_;

    nlohmann::json framing_definition_;
    std::unique_ptr<FrameParser> frame_parser_;
};
}

#endif // JASTERIX_H
