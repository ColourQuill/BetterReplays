#ifndef __SCREEN_CAPTURE_HPP__
#define __SCREEN_CAPTURE_HPP__

// std
#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <mutex>
#include <atomic>

// better replays
#include <frame.hpp>
#include <settings.hpp>

using FrameCallback = std::function<void(const Frame&)>;
using EncoderCallback = std::function<void(uint32_t, uint32_t)>;

class ScreenCapture {
    public:
        virtual ~ScreenCapture() = default;

        virtual bool init() = 0;
        virtual bool start() = 0;
        virtual void stop() = 0;

        void setEncoderInitCallback(EncoderCallback encoderCallback) { onEncoderInit = encoderCallback; }
        void setFrameCallback(FrameCallback frameCallback) { onFrame = std::move(frameCallback); }
        void setSettings(Settings* settings) { this->settings = settings; }
        
        bool hasFrame() const { return frameReady.load(); }
        void withLatestFrame(std::function<void(const Frame&)> fn);
        void pushFrame(Frame frame);

        bool hasEncoderInitialized();
        void initEncoder(int srcWidth, int srcHeight);

        static ScreenCapture* create();
    protected:
        FrameCallback onFrame;
        EncoderCallback onEncoderInit;
        bool encoderInitialized{false};

        Frame latestFrame;
        std::mutex frameMutex;
        std::atomic<bool> frameReady{false};

        Settings* settings = nullptr;
};

#endif