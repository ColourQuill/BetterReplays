#include <screen_capture.hpp>

// std
#include <cstring>

#ifdef _WIN32
    #include <windows_screen_capture.hpp>
#elif __APPLE__
    #include <mac_screen_capture.hpp>
#elif __linux__
    #include <linux_screen_capture.hpp>
#endif

ScreenCapture* ScreenCapture::create() {
#ifdef _WIN32
    return new WindowsScreenCapture();
#elif __APPLE__
    return new MacScreenCapture();
#elif __linux__
    return new LinuxScreenCapture();
#else
    static_assert(false, "Capture creation failed: Unsupported platform.");
#endif
}

void ScreenCapture::pushFrame(Frame frame) {
    if (onFrame) {
        onFrame(frame);
    }
    {
        std::lock_guard<std::mutex> lock(frameMutex);
        latestFrame.width       = frame.width;
        latestFrame.height      = frame.height;
        latestFrame.stride      = frame.stride;
        latestFrame.timestampMS = frame.timestampMS;
        latestFrame.data.resize(frame.data.size());
        std::memcpy(latestFrame.data.data(), frame.data.data(), frame.data.size());
        frameReady.store(true);
    }
}

void ScreenCapture::withLatestFrame(std::function<void(const Frame&)> fn) {
    std::lock_guard<std::mutex> lock(frameMutex);
    if (frameReady.load()) {
        fn(latestFrame);
        frameReady.store(false);
    }
}

bool ScreenCapture::hasEncoderInitialized() {
    return !onEncoderInit || encoderInitialized;
}

void ScreenCapture::initEncoder(int srcWidth, int srcHeight) {
    encoderInitialized = true;
    onEncoderInit(srcWidth, srcHeight);
}