#include "jasterix.h"
#include "jasterix_files.h"
#include "frame_parser.h"

#include <exception>
#include <fstream>

using namespace nlohmann;

namespace jASTERIX {

using namespace Files;

jASTERIX::jASTERIX(const std::string& filename, const std::string& definition_path, const std::string& framing)
    : filename_(filename), definition_path_(definition_path), framing_(framing)
{
    if (!fileExists(filename_))
        throw std::invalid_argument ("jASTERIX called with non-existing file '"+filename_+"'");

    if (!directoryExists(definition_path_))
        throw std::invalid_argument ("jASTERIX called with non-existing definition path '"+definition_path_+"'");

    if (!directoryExists(definition_path_+"/framings"))
        throw std::invalid_argument ("jASTERIX called with incorrect definition path '"+definition_path_
                                     +"', framings are missing");

    if (!directoryExists(definition_path_+"/categories"))
        throw std::invalid_argument ("jASTERIX called with incorrect definition path '"+definition_path_
                                     +"', categories are missing");

    if (!fileExists(definition_path_+"/framings/"+framing_+".json"))
        throw std::invalid_argument ("jASTERIX called with unknown framing '"+framing_+"'");

    try
    {
        std::ifstream ifs(definition_path_+"/framings/"+framing_+".json");
        framing_definition_ = json::parse(ifs);
    }
    catch (json::exception& e)
    {
        throw std::runtime_error ("jASTERIX parsing error in framing definition '"+framing_+"': "+e.what());
    }

    frame_parser_.reset(new FrameParser(framing_definition_));
}

}
