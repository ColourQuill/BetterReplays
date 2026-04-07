#ifndef __ENCODED_PACKET_HPP__
#define __ENCODED_PACKET_HPP__

// std
#include <vector>
#include <cstdint>

struct EncodedPacket {
    std::vector<uint8_t> data;
    int64_t presentationTimestamp;
    int64_t decodeTimestamp;
    bool isKeyframe;
};

#endif