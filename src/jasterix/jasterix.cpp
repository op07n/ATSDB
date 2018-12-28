#include "jasterix.h"
#include "jasterix_files.h"
#include "frameparser.h"

#include <exception>
#include <fstream>
#include <iostream>

using namespace nlohmann;

namespace jASTERIX {

using namespace Files;
using namespace std;

jASTERIX::jASTERIX(const std::string& filename, const std::string& definition_path, const std::string& framing,
                   bool debug)
    : filename_(filename), definition_path_(definition_path), framing_(framing), debug_(debug)
{
    //cout << "constructing jASTERIX" << endl;

    // check and open file
    if (!fileExists(filename_))
        throw invalid_argument ("jASTERIX called with non-existing file '"+filename_+"'");

    file_size_ = fileSize (filename_);

    if (!file_size_)
        throw invalid_argument ("jASTERIX called with empty file '"+filename_+"'");

    if (debug_)
        cout << "jASTERIX: file " << filename_ << " size " << file_size_ << endl;

    file_.open(filename_, file_size_);

    if(!file_.is_open())
        throw runtime_error ("jASTERIX unable to map file '"+filename_+"'");

    // check definitions
    if (!directoryExists(definition_path_))
        throw invalid_argument ("jASTERIX called with non-existing definition path '"+definition_path_+"'");

    if (!directoryExists(definition_path_+"/framings"))
        throw invalid_argument ("jASTERIX called with incorrect definition path '"+definition_path_
                                     +"', framings are missing");

    if (!directoryExists(definition_path_+"/categories"))
        throw invalid_argument ("jASTERIX called with incorrect definition path '"+definition_path_
                                     +"', categories are missing");

    if (!fileExists(definition_path_+"/framings/"+framing_+".json"))
        throw invalid_argument ("jASTERIX called with unknown framing '"+framing_+"'");

    if (!fileExists(definition_path_+"/record_definition.json"))
        throw invalid_argument ("jASTERIX called without asterix record definition");

    try // create framing definition and parser
    {
        ifstream ifs(definition_path_+"/framings/"+framing_+".json");
        framing_definition_ = json::parse(ifs);

        ifstream ifs2(definition_path_+"/record_definition.json");
        record_definition_ = json::parse(ifs2);

        frame_parser_.reset(new FrameParser(framing_definition_, record_definition_));
    }
    catch (json::exception& e)
    {
        throw runtime_error ("jASTERIX parsing error in framing definition '"+framing_+"': "+e.what());
    }
}

jASTERIX::~jASTERIX()
{
    if(file_.is_open())
        file_.close();
}

size_t jASTERIX::scopeFrames()
{
    assert (frame_parser_);
    assert (file_.is_open());
    assert (json_data_ == nullptr);

    //cout << "jASTERIX scoping frames" << endl;

    frame_parser_->scopeFrames(file_.data(), 0, file_size_, json_data_, debug_);

    if (json_data_.find("frames") == json_data_.end())
        throw runtime_error ("jASTERIX scoped frames information contains no frames");

    if (!json_data_.at("frames").is_array())
        throw runtime_error ("jASTERIX scoped frames information is not array");

    cout << "jASTERIX frame scoping JSON result contains " << json_data_.at("frames").size() << " frames" << endl;

    return json_data_.at("frames").size();
}

void jASTERIX::decodeFrames()
{
    assert (frame_parser_);
    assert (file_.is_open());
    assert (json_data_ != nullptr);

    frame_parser_->decodeFrames(file_.data(), json_data_, debug_);
}

void jASTERIX::printData()
{
    cout << "jASTERIX data: '" << json_data_.dump(4) << "'" << endl;
}

}
