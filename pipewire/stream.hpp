#ifndef __STREAM_HPP__
#define __STREAM_HPP__

// better replays
#include <format.hpp>
#include <crop.hpp>
#include <cursor.hpp>
#include <resolution.hpp>
#include <framerate.hpp>

// pipewire
#include <spa/param/video/format-utils.h>
#include <spa/pod/builder.h>

// drm
#include <drm/drm_fourcc.h>

// debug
#include <SDL2/SDL.h>
#include <mutex>

// forward declarations
struct Instance;

struct Stream {
    Instance* instance;
    
    pw_stream* stream;
    spa_hook streamListener;
    spa_source* reneg;

    spa_video_info_raw videoFormat;

    spa_meta_videotransform_value transform;

    Crop crop;
    Cursor cursor;

    bool negotiated;

    std::vector<FormatInfo> formatInfo;

    Resolution resolution;
    Framerate framerate;

    Frame frameBuffer;

    std::chrono::steady_clock::time_point lastFrametime;
    double accumulator = 0.0;
};

struct StreamInfo {
    const char* streamName;
    pw_properties* streamProperties;
    bool cursorVisible;
    Resolution* resolution = nullptr;
    Framerate* framerate = nullptr;
};

#endif