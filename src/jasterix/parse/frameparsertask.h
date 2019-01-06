#ifndef FRAMEPARSERTASK_H
#define FRAMEPARSERTASK_H

#include "jasterix.h"
#include "frameparser.h"

#include <tbb/tbb.h>

#include <exception>

class FrameParserTask : public tbb::task
{
public:
    FrameParserTask (jASTERIX::jASTERIX& jasterix, jASTERIX::FrameParser& frame_parser, const nlohmann::json& header,
                     const char* data, size_t index, size_t size, bool debug)
        : jasterix_(jasterix), frame_parser_(frame_parser), header_(header), data_(data), index_(index), size_(size),
          debug_(debug)
    {}

    /*override*/ tbb::task* execute()
    {
        while (!frame_parser_.done() || size_-index_ > 0)
        {
            nlohmann::json data_chunk = header_; // copy header

            assert (index_ < size_);

            index_ += frame_parser_.parseFrames(data_, index_, size_, data_chunk, 20000, debug_);

            assert (data_chunk != nullptr);

            if (data_chunk.find("frames") == data_chunk.end())
                throw std::runtime_error ("jASTERIX scoped frames information contains no frames");

            if (!data_chunk.at("frames").is_array())
                throw std::runtime_error ("jASTERIX scoped frames information is not array");

            jasterix_.addDataChunk(data_chunk);
        }

        return nullptr; // or a pointer to a new task to be executed immediately
    }

private:
    jASTERIX::jASTERIX& jasterix_;
    jASTERIX::FrameParser& frame_parser_;
    const nlohmann::json& header_;
    const char* data_;
    size_t index_;
    size_t size_;
    bool debug_;
};

#endif // FRAMEPARSERTASK_H
