#ifndef __MUXER_HPP__
#define __MUXER_HPP__

// std
#include <string>
#include <vector>

// better replays
#include <encoded_packet.hpp>
#include <settings.hpp>
#include <codec_parameters.hpp>

class Muxer {
    public:
        virtual ~Muxer() = default;
        

        virtual bool save(const std::string& filename, const std::vector<EncodedPacket>& packets, CodecParameters codecParams) = 0;

        void setSettings(Settings* settings) { this->settings = settings; }

        static Muxer* create();
    protected:
        Settings* settings = nullptr;
};

#endif