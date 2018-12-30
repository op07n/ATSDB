#include "jasterix.h"
#include "jasterix_files.h"
#include "jasterix_logger.h"
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
    // check and open file
    if (!fileExists(filename_))
        throw invalid_argument ("jASTERIX called with non-existing file '"+filename_+"'");

    file_size_ = fileSize (filename_);

    if (!file_size_)
        throw invalid_argument ("jASTERIX called with empty file '"+filename_+"'");

    if (debug_)
        loginf << "jASTERIX: file " << filename_ << " size " << file_size_;

    file_.open(filename_, file_size_);

    if(!file_.is_open())
        throw runtime_error ("jASTERIX unable to map file '"+filename_+"'");

    // check framing definitions
    if (!directoryExists(definition_path_))
        throw invalid_argument ("jASTERIX called with non-existing definition path '"+definition_path_+"'");

    if (!directoryExists(definition_path_+"/framings"))
        throw invalid_argument ("jASTERIX called with incorrect definition path '"+definition_path_
                                     +"', framings are missing");

    if (!fileExists(definition_path_+"/framings/"+framing_+".json"))
        throw invalid_argument ("jASTERIX called with unknown framing '"+framing_+"'");

    if (!fileExists(definition_path_+"/data_block_definition.json"))
        throw invalid_argument ("jASTERIX called without asterix data block definition");

    // check asterix definitions

    if (!directoryExists(definition_path_+"/categories"))
        throw invalid_argument ("jASTERIX called with incorrect definition path '"+definition_path_
                                     +"', categories are missing");

    if (!fileExists(definition_path_+"/categories/categories.json"))
        throw invalid_argument ("jASTERIX called without asterix categories list definition");

    try // create framing definition
    {
        framing_definition_ = json::parse(ifstream(definition_path_+"/framings/"+framing_+".json"));
    }
    catch (json::exception& e)
    {
        throw runtime_error ("jASTERIX parsing error in framing definition '"+framing_+"': "+e.what());
    }

    try // asterix record definition
    {
        record_definition_ = json::parse(ifstream(definition_path_+"/data_block_definition.json"));
    }
    catch (json::exception& e)
    {
        throw runtime_error (string{"jASTERIX parsing error in asterix data block definition: "}+e.what());
    }

    try // asterix categories list definition
    {
        asterix_list_definition_ = json::parse(ifstream(definition_path_+"/categories/categories.json"));
    }
    catch (json::exception& e)
    {
        throw runtime_error (string{"jASTERIX parsing error in asterix categories list definition: "}+e.what());
    }

    if (!asterix_list_definition_.is_object())
        throw invalid_argument ("jASTERIX called with non-object asterix categories list definition");

    try // asterix category definitions
    {
        std::string cat_str;
        std::string file_str;
        int cat;

        for (auto ast_def_it = asterix_list_definition_.begin(); ast_def_it != asterix_list_definition_.end();
             ++ast_def_it)
        {
            cat = -1;
            cat_str = ast_def_it.key();
            file_str = ast_def_it.value();
            cat = stoi(cat_str);

            if (cat < 0 || cat > 255 || asterix_category_definitions_.count(cat) != 0)
                throw invalid_argument ("jASTERIX called with wrong asterix category '"+cat_str+"' in list definition");

            if (debug)
                loginf << "jASTERIX found asterix category " << cat << " definition in '" << file_str << "'";

            try
            {
                if (debug)
                   loginf << "jASTERIX loading file from path '"+definition_path_+"/categories/"+file_str << "'";

                asterix_category_definitions_[cat] =
                        json::parse(ifstream(definition_path_+"/categories/"+file_str));
            }
            catch (json::exception& e)
            {
                throw runtime_error ("jASTERIX parsing error in asterix category "+cat_str
                                     +" definition '"+file_str+"':"+e.what());
            }
        }
    }
    catch (json::exception& e)
    {
        throw runtime_error (string{"jASTERIX parsing error in asterix category definitions: "}+e.what());
    }

    frame_parser_.reset(new FrameParser(framing_definition_, record_definition_, asterix_category_definitions_));
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

    if (debug_)
        loginf << "jASTERIX frame scoping JSON result contains " << json_data_.at("frames").size() << " frames";

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
    loginf << "jASTERIX data: '" << json_data_.dump(4) << "'";
}

}
