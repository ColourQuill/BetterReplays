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

using FrameCallback = std::function<void(const Frame&)>;

class ScreenCapture {
    public:
        virtual ~ScreenCapture() = default;

        virtual bool init() = 0;
        virtual bool start() = 0;
        virtual void stop() = 0;

        void setFrameCallback(FrameCallback frameCallback) { onFrame = std::move(frameCallback); }
        bool hasFrame() const { return frameReady.load(); }
        void withLatestFrame(std::function<void(const Frame&)> fn);
        void pushFrame(Frame frame);

        static ScreenCapture* create();
    protected:
        FrameCallback onFrame;
        Frame latestFrame;
        std::mutex frameMutex;
        std::atomic<bool> frameReady{false};
};

#endif