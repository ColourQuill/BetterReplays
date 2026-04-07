#include <encoder.hpp>

#include <ffmpeg_encoder.hpp>

Encoder* Encoder::create() {
    return new FFmpegEncoder();
}