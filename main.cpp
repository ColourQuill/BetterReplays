// better replays
#include <screen_capture.hpp>
#include <encoder.hpp>
#include <buffer.hpp>
#include <muxer.hpp>
#include <logger.hpp>

// std
#include <thread>
#include <chrono>

// debug
#include <SDL2/SDL.h>

// NOTE
// Currently exists a slow memory leak overtime. Do intend to fix this.

int main() {
    ScreenCapture* capture = ScreenCapture::create();
    Encoder* encoder = Encoder::create();
    Muxer* muxer = Muxer::create();

    Buffer* buffer = new Buffer(30000);

    encoder->init(1920, 1080, 60);

    encoder->setPacketCallback([&](const EncodedPacket& packet) {
        buffer->addPacket(packet);
    });

    capture->setFrameCallback([&](const Frame& frame) {
        encoder->encode(frame);
    });

    if (!capture->init()) {
        Logger::logError("Main", "Failed to initialize capture.");
        return 1;
    }
    if (!capture->start()) {
        Logger::logError("Main", "Failed to start capture.");
        return 1;
    }

    int waitTime = 10;
    Logger::logInfo("Main", "Waiting for " + std::to_string(waitTime) + " seconds.");
    std::this_thread::sleep_for(std::chrono::seconds(waitTime));

    std::string filename = "replay.mp4";

    Logger::logInfo("Main", "Saving replay...");
    std::vector<EncodedPacket> replaySnapshot = buffer->getSnapshot();
    bool success = muxer->save(filename, replaySnapshot, 1920, 1080, 60);

    if (success) {
        Logger::logInfo("Main", "Replay saved successfully to -> " + filename);
    } else {
        Logger::logError("Main", "Failed to save replay.");
    }

    while (!capture->hasFrame()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    capture->stop();
    delete capture;
    delete encoder;
    delete buffer;
    delete muxer;

    Logger::logInfo("Main", "Shutdown complete.");
    return 0;
}