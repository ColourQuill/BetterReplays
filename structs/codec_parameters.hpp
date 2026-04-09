#ifndef __CODEC_PARAMETERS_HPP__
#define __CODEC_PARAMETERS_HPP__

// std
#include <cstdint>
#include <vector>

struct CodecParameters {
    int width, height, fps;
    std::vector<uint8_t> extraData;
};

#endif