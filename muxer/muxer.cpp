#include <muxer.hpp>

#include <ffmpeg_muxer.hpp>

Muxer* Muxer::create() {
    return new FFmpegMuxer();
}