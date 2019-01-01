#ifndef ITEMPARSING_H
#define ITEMPARSING_H

#include <cstddef>
#include <sstream>

#include "json.hpp"

namespace jASTERIX
{
// always return number of parsed bytes

// parses an item based on type, by calling the following functions
size_t parseItem (const nlohmann::json& item_definition, const char* data, size_t index, size_t size,
                  size_t current_parsed_bytes, nlohmann::json& target, nlohmann::json& parent, bool debug);

// parses fixed number of bytes, can distinguish data tyes
size_t parseFixedBytesItem (const std::string& name, const std::string& type, const nlohmann::json& item_definition,
                            const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                            nlohmann::json& target, nlohmann::json& parent, bool debug);

// calculates the index and length based on other decoded variable
size_t parseDynamicBytesItem (const std::string& name, const std::string& type, const nlohmann::json& item_definition,
                              const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                              nlohmann::json& target, nlohmann::json& parent, bool debug);

// decodes a field specification/availablity field (ending with extend bit), and list of items
size_t parseCompoundItem (const std::string& name, const std::string& type, const nlohmann::json& item_definition,
                          const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                          nlohmann::json& target, nlohmann::json& parent, bool debug);

// parses all bits per byte into array<bool>, the last of each byte signifying the extension into next byte
size_t parseExtendtableBitsItem (const std::string& name, const std::string& type,
                                 const nlohmann::json& item_definition,const char* data, size_t index, size_t size,
                                 size_t current_parsed_bytes, nlohmann::json& target, nlohmann::json& parent,
                                 bool debug);

// parses item into array, extend into next has to signified by bool extend
size_t parseExtendtableItem (const std::string& name, const std::string& type,
                             const nlohmann::json& item_definition,const char* data, size_t index, size_t size,
                             size_t current_parsed_bytes, nlohmann::json& target, nlohmann::json& parent,
                             bool debug);

size_t parseFixedBitfieldItem (const std::string& name, const std::string& type,
                               const nlohmann::json& item_definition,const char* data, size_t index, size_t size,
                               size_t current_parsed_bytes, nlohmann::json& target, nlohmann::json& parent,
                               bool debug);

size_t parseFixedBitsItem (const std::string& name, const std::string& type, const nlohmann::json& item_definition,
                           const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                           nlohmann::json& target, nlohmann::json& parent, bool debug);

size_t parseOptionalItem (const std::string& name, const std::string& type, const nlohmann::json& item_definition,
                           const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                           nlohmann::json& target, nlohmann::json& parent, bool debug);

size_t parseRepetitiveItem (const std::string& name, const std::string& type, const nlohmann::json& item_definition,
                           const char* data, size_t index, size_t size, size_t current_parsed_bytes,
                           nlohmann::json& target, nlohmann::json& parent, bool debug);

bool variableHasValue (const nlohmann::json& data, const std::string& variable_name,
                       const nlohmann::json& variable_value);

inline unsigned char reverseBits(unsigned char b)
{
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;

   return b;
}

inline std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return std::move(elems);
}

}

#endif // ITEMPARSING_H
