#ifndef JASTERIX_H
#define JASTERIX_H

#include <string>
#include <map>
#include <boost/iostreams/device/mapped_file.hpp>
#include "boost/thread/mutex.hpp"

#include "json.hpp"
#include "frameparser.h"

namespace jASTERIX {

class jASTERIX
{
public:
    jASTERIX(const std::string& filename, const std::string& definition_path, const std::string& framing_str,
             bool print, bool debug);
    virtual ~jASTERIX();

    // returns number of decoded records
    void decode ();

    size_t numFrames() const;
    size_t numRecords() const;

    void addDataChunk (nlohmann::json& data_chunk);

private:
    std::string filename_;
    std::string definition_path_;
    std::string framing_;
    bool print_ {false};
    bool debug_ {false};

    nlohmann::json framing_definition_;
    nlohmann::json data_block_definition_;
    nlohmann::json asterix_list_definition_;
    std::map<unsigned int, nlohmann::json> asterix_category_definitions_;
    std::unique_ptr<FrameParser> frame_parser_;

    size_t file_size_{0};
    boost::iostreams::mapped_file_source file_;

    boost::mutex data_mutex_;
    std::vector<nlohmann::json> data_chunks_;

    size_t num_frames_{0};
    size_t num_records_{0};
};
}

#endif // JASTERIX_H
