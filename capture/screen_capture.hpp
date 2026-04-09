#ifndef __SCREEN_CAPTURE_HPP__
#define __SCREEN_CAPTURE_HPP__

// std
#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <mutex>
#include <atomic>
#include <thread>
#include <queue>
#include <condition_variable>

// better replays
#include <frame.hpp>
#include <settings.hpp>

using FrameCallback = std::function<void(const Frame&)>;
using EncoderCallback = std::function<void(uint32_t, uint32_t, PixelFormat)>;

class ScreenCapture {
    public:
        virtual ~ScreenCapture() = default;

        virtual bool init() = 0;
        virtual bool start() = 0;
        virtual void stop() = 0;

        void setDisplayConfig(const DisplayConfig& displayConfig) { this->displayConfig = displayConfig; }
        void setEncoderInitCallback(EncoderCallback encoderCallback) { onEncoderInit = encoderCallback; }
        void setFrameCallback(FrameCallback frameCallback) { onFrame = std::move(frameCallback); }
        void setSettings(Settings* settings) { this->settings = settings; }
        
        bool hasFrame() const { return frameReady.load(); }
        void withLatestFrame(std::function<void(const Frame&)> fn);
        void pushFrame(Frame frame);

        bool hasEncoderInitialized();
        void initEncoder(int srcWidth, int srcHeight, PixelFormat pixelFormat);

        bool isRunning();

        void pushFrameAsync(const Frame& frame);
        void encodeLoop();

        static ScreenCapture* create();
    protected:
        FrameCallback onFrame;
        EncoderCallback onEncoderInit;
        bool encoderInitialized{false};
        bool running{false};

        Frame latestFrame;
        std::mutex frameMutex;
        std::atomic<bool> frameReady{false};

        DisplayConfig displayConfig;
        Settings* settings = nullptr;

        std::thread encodeThread;
        std::queue<Frame> frameQueue;
        std::mutex queueMutex;
        std::condition_variable queueCV;

        
};

#endif