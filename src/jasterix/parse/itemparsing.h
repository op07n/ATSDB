#ifndef ITEMPARSING_H
#define ITEMPARSING_H

#include <cstddef>
#include <sstream>

#include "json.hpp"
#include "jasterix_logger.h"

#include <string>
#include <cassert>
#include <exception>
#include <bitset>

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

// skips fixed number of bytes
size_t parseSkipBytesItem (const std::string& name, const std::string& type, const nlohmann::json& item_definition,
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

template <typename T>
inline void parseFixedBitsItem (const std::string& name, const std::string& type, const nlohmann::json& item_definition,
                           T data, nlohmann::json& target, bool debug)
{
    if (debug)
    {
        assert (type == "fixed_bits");
        loginf << "parsing fixed bits item '" << name << "'";
    }

    if (debug && item_definition.find("start_bit") == item_definition.end())
        throw std::runtime_error ("parsing fixed byte bitfield item '"+name+"' without start bit");

    unsigned int start_bit = item_definition.at("start_bit");

    if (debug && item_definition.find("bit_length") == item_definition.end())
        throw std::runtime_error ("parsing fixed byte bitfield item '"+name+"' without bit length");

    unsigned int bit_length = item_definition.at("bit_length");

    if (debug)
        loginf << "parsing fixed bits item '" << name << "'"
               << " data '" << std::hex << (size_t) data << "'"
               << " with start bit " << start_bit << " length " << bit_length;

//    T bitmask {1};
//    bitmask <<= start_bit+bit_length-1;

//    bool bit_set {false};
//    T value {0};

//    for (unsigned cnt=0; cnt < bit_length; ++cnt)
//    {
//        value <<= 1;
//        bit_set = data & bitmask;
//        value |= bit_set;

//        if (debug)
//            loginf << "parsing fixed bits item '" << name << "' with bit " << cnt
//                   << " bitmask " << (size_t) bitmask << " set " << bit_set << " value " << (size_t) value;

//        bitmask >>= 1;
//    }

//    std::bitset<sizeof(T)*8> bitmask(0);
//    bitmask = ((bitmask.flip() << bit_length).flip() << start_bit).flip();

    T bitmask {1};
    for (unsigned cnt=0; cnt < bit_length; ++cnt)
    {
        bitmask <<= 1;
        bitmask += 1;
    }
    bitmask <<= start_bit;

    if (debug)
        loginf << "parsing fixed bits item '" << name << "' bitmask " << (size_t) bitmask;

    T value = data & bitmask;

    value >>= start_bit;

    if (debug)
        loginf << "parsing fixed bits item '" << name << "' with start bit " << start_bit
               << " length " << bit_length << " value " << (size_t) value;

    target = value;
}

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
