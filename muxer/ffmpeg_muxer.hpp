#ifndef __FFMPEG_MUXER_HPP__
#define __FMMPEG_MUXER_HPP__

// better replays
#include <muxer.hpp>
#include <encoded_packet.hpp>

// std
#include <vector>
#include <string>

// forward declarations
struct AVFormatContext;
struct AVStream;

class FFmpegMuxer : public Muxer {
    public:
        FFmpegMuxer() = default;
        ~FFmpegMuxer() override = default;

        bool save(const std::string& filename, const std::vector<EncodedPacket>& packets, int width, int height, int fps) override;
    private:
        AVFormatContext* formatContext = nullptr;
        AVStream* videoStream = nullptr;
};

#endif