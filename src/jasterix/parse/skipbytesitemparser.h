#ifndef SKIPBYTESITEMPARSER_H
#define SKIPBYTESITEMPARSER_H

#include "itemparser.h"

namespace jASTERIX
{
// skips fixed number of bytes
class SkipBytesItemParser : public ItemParser
{
public:
    SkipBytesItemParser (const nlohmann::json& item_definition);
    virtual ~SkipBytesItemParser() {}

    virtual size_t parseItem (const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                              nlohmann::json& target, bool debug) override;
protected:
    unsigned int length_{0};
};

}

#endif // SKIPBYTESITEMPARSER_H
