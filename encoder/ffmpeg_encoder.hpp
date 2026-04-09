#ifndef __FFMPEG_ENCODER_HPP__
#define __FFMPEG_ENCODER_HPP__

// better replays
#include <encoder.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class FFmpegEncoder : public Encoder {
    public:
        FFmpegEncoder() = default;
        ~FFmpegEncoder();

        bool init(int srcWidth, int srcHeight, PixelFormat srcFormat, int dstWidth, int dstHeight, int fps) override;
        bool encode(const Frame& frame) override;
        void flush() override;
    private:
        AVCodecContext* codecContext = nullptr;
        AVFrame* avFrame = nullptr;
        AVPacket* avPacket = nullptr;
        SwsContext* swsContext = nullptr;

        int64_t frameCounter = 0;

        std::string codecName;
        bool isHardware = false;

        AVBufferRef* hwDeviceCtx = nullptr;

        void initSoftware();
        void initNvenc();
        bool initVaapi(int width, int height);
};

#endif