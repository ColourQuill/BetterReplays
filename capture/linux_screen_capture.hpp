#ifndef __LINUX_SCREEN_CAPTURE_HPP__
#define __LINUX_SCREEN_CAPTURE_HPP__

#ifdef __linux__

#include <screen_capture.hpp>
#include <pipewire/pipewire.h>
#include <spa/param/video/format-utils.h>

#include <instance.hpp>
#include <stream.hpp>

class LinuxScreenCapture : public ScreenCapture {
    public:
        ~LinuxScreenCapture() override { cleanup(); }

        bool init() override;
        bool start() override;
        void stop() override;
    private:
        Instance instance{};

        StreamInfo getStreamInfo();

        bool pipewireConnectFD(int fd);
        bool pipewireConnectStream(int node, StreamInfo& streamInfo);

        std::vector<FormatInfo> initFormatInfo();

        void freeStream(Stream* stream);
        void cleanup();
};

#endif

#endif