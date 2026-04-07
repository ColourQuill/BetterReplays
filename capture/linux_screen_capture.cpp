#ifdef __linux__

#include <linux_screen_capture.hpp>

// better replays
#include <portal.hpp>
#include <format.hpp>
#include <logger.hpp>

// pipewire
#include <spa/param/video/format-utils.h>
#include <spa/pod/builder.h>

// drm
#include <drm/drm_fourcc.h>

// std
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>

// debug
#include <iostream>

bool LinuxScreenCapture::init() {
    pw_init(nullptr, nullptr);

    int fd, node;
    Portal::request(fd, node);

    if (!pipewireConnectFD(fd)) {
        return false;
    }

    StreamInfo streamInfo = getStreamInfo();

    if (!pipewireConnectStream(node, streamInfo)) {
        return false;
    }

    return true;
}

bool LinuxScreenCapture::start() {
    if (instance.streams.empty()) {
        Logger::logError("pipewire", "No streams to start.");
        return false;
    }

    pw_thread_loop_lock(instance.threadLoop);

    for (Stream* stream : instance.streams) {
        int ret = pw_stream_set_active(stream->stream, true);
        if (ret < 0) {
            Logger::logError("pipewire", "Failed to activate stream -> " + std::to_string(ret));
            pw_thread_loop_unlock(instance.threadLoop);
            return false;
        }
        Logger::logInfo("pipewire", "Activated stream -> " + Logger::ptrToStr(stream->stream));
    }

    pw_thread_loop_unlock(instance.threadLoop);

    return true;
}

void LinuxScreenCapture::stop() {
    if (instance.streams.empty()) {
        return;
    }
    pw_thread_loop_lock(instance.threadLoop);

    for (Stream* stream : instance.streams) {
        pw_stream_set_active(stream->stream, false);
        stream->negotiated = false;
    }

    pw_thread_loop_unlock(instance.threadLoop);
}

static void onParamChanged(void* userData, uint32_t id, const struct spa_pod* param) {
    Stream* stream = static_cast<Stream*>(userData);

    if (!param || id != SPA_PARAM_Format) {
        return;
    }

    spa_video_info info = {};
    if (spa_format_parse(param, &info.media_type, &info.media_subtype) < 0) {
        Logger::logError("pipewire", "Failed to parse media type.");
        return;
    }

    if (info.media_subtype == SPA_MEDIA_SUBTYPE_raw) {
        if (spa_format_video_raw_parse(param, &stream->videoFormat) < 0) {
            Logger::logError("pipewire", "Failed to parse raw video format.");
            return;
        }
    }

    Logger::logInfo("pipewire", "Negotiated format: " + std::to_string(stream->videoFormat.format)
        + " " + std::to_string(stream->videoFormat.size.width)
        + "x" + std::to_string(stream->videoFormat.size.height));

    stream->negotiated = true;

    uint8_t buffer[1024];
    spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

    int32_t bpp = 4;
    int32_t stride = stream->videoFormat.size.width * bpp;
    int32_t size = stride * stream->videoFormat.size.height;

    const spa_pod* params[1];
    params[0] = (spa_pod*)spa_pod_builder_add_object(&b,
        SPA_TYPE_OBJECT_ParamBuffers, SPA_PARAM_Buffers,
        SPA_PARAM_BUFFERS_buffers,  SPA_POD_CHOICE_RANGE_Int(4, 1, 32),
        SPA_PARAM_BUFFERS_blocks,   SPA_POD_Int(1),
        SPA_PARAM_BUFFERS_size,     SPA_POD_Int(size),
        SPA_PARAM_BUFFERS_stride,   SPA_POD_Int(stride)
    );

    pw_stream_update_params(stream->stream, params, 1);
}

static void onProcess(void* userData) {
    Stream* stream = static_cast<Stream*>(userData);

    if (!stream->negotiated) {
        return;
    }

    // FIX THIS TO WORK WITH FPS AND CHANGE RESOLUTION SETTINGS

    auto now = std::chrono::steady_clock::now();
    
    static auto lastFrameTime = now;
    static double accumulator = 0.0; 
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - lastFrameTime).count();
    lastFrameTime = now;
    
    accumulator += (duration / 1000.0);

    const double frameTarget = 1000.0 / 60.0;

    if (accumulator < frameTarget) {
        pw_buffer* pwBuffer = pw_stream_dequeue_buffer(stream->stream);
        if (pwBuffer) pw_stream_queue_buffer(stream->stream, pwBuffer);
        return;
    }

    accumulator -= frameTarget;

    pw_buffer* pwBuffer = pw_stream_dequeue_buffer(stream->stream);
    if (!pwBuffer) {
        Logger::logError("pipewire", "No buffer available.");
        return;
    }

    spa_buffer* spaBuffer = pwBuffer->buffer;
    if (!spaBuffer || !spaBuffer->datas[0].data) {
        pw_stream_queue_buffer(stream->stream, pwBuffer);
        return;
    }

    void* data     = spaBuffer->datas[0].data;
    uint32_t size  = spaBuffer->datas[0].chunk->size;
    uint32_t width  = stream->videoFormat.size.width;
    uint32_t height = stream->videoFormat.size.height;
    int stride = width * 4;

    Frame& frame = stream->frameBuffer;
    frame.width = width;
    frame.height = height;
    frame.stride = stride;
    frame.timestampMS = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (size > 0) {
        frame.data.resize(size);
        std::memcpy(frame.data.data(), data, size);

        stream->instance->capture->pushFrame(frame);
    }

    pw_stream_queue_buffer(stream->stream, pwBuffer);
}

static void onStateChanged(void* userData, pw_stream_state old, pw_stream_state state, const char* error) {
    Stream* stream = static_cast<Stream*>(userData);
    Logger::logInfo("pipewire", std::string("Stream state changed: ") + pw_stream_state_as_string(state));
    if (state == PW_STREAM_STATE_ERROR) {
        Logger::logError("pipewire", std::string("Stream error: ") + (error ? error : "unknown"));
    }
}

static void onCoreDone(void* userData, uint32_t id, int seq) {
    Instance* instance = static_cast<Instance*>(userData);
    if (id == PW_ID_CORE && seq == instance->sync_id) {
        pw_thread_loop_signal(instance->threadLoop, false);
    }
}
const static spa_pod* buildFormat(spa_pod_builder* builder, FormatInfo& format, bool modifiers) {
    spa_pod_frame formatFrame;
    spa_pod_frame modifierFrame;

    spa_rectangle resolution = SPA_RECTANGLE(1920, 1080);
    spa_rectangle minRes = SPA_RECTANGLE(1, 1);
    spa_rectangle maxRes = SPA_RECTANGLE(8192, 4320);
    spa_fraction framerate = SPA_FRACTION(60, 1);
    spa_fraction minFps = SPA_FRACTION(0, 1);
    spa_fraction maxFps = SPA_FRACTION(360, 1);

    spa_pod_builder_push_object(builder, &formatFrame, SPA_TYPE_OBJECT_Format, SPA_PARAM_EnumFormat);

    spa_pod_builder_add(builder, SPA_FORMAT_mediaType, SPA_POD_Id(SPA_MEDIA_TYPE_video), 0);
    spa_pod_builder_add(builder, SPA_FORMAT_mediaSubtype, SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw), 0);
    spa_pod_builder_add(builder, SPA_FORMAT_VIDEO_format, SPA_POD_Id(format.spaFormat), 0);

    if (modifiers) {
        spa_pod_builder_prop(builder, SPA_FORMAT_VIDEO_modifier, SPA_POD_PROP_FLAG_MANDATORY | SPA_POD_PROP_FLAG_DONT_FIXATE);
        spa_pod_builder_push_choice(builder, &modifierFrame, SPA_CHOICE_Enum, 0);
        spa_pod_builder_long(builder, format.modifiers[0]);
        for (size_t i = 0; i < format.modifiers.size(); ++i) {
            spa_pod_builder_long(builder, format.modifiers[i]);
        }
        spa_pod_builder_pop(builder, &modifierFrame);
    }

    spa_pod_builder_add(
        builder, 
        SPA_FORMAT_VIDEO_size, SPA_POD_CHOICE_RANGE_Rectangle(&resolution, &minRes, &maxRes), 
        SPA_FORMAT_VIDEO_framerate, SPA_POD_CHOICE_RANGE_Fraction(&framerate, &minFps, &maxFps), 
        0
    );

    return (const spa_pod*)spa_pod_builder_pop(builder, &formatFrame);
}
static bool buildFormatParams(Stream* stream, spa_pod_builder* builder, const spa_pod**& params, uint32_t& numParams) {
    if (stream->formatInfo.empty()) {
        Logger::logError("pipewire", "No format available for stream.");
        return false;
    }

    params = new const spa_pod*[stream->formatInfo.size() * 2];
    numParams = 0;
    
    for (auto& format : stream->formatInfo) {
        if (format.modifiers.empty()) {
            continue;
        }
        params[numParams++] = buildFormat(builder, format, true);
    }

    for (auto& format : stream->formatInfo) {
        params[numParams++] = buildFormat(builder, format, false);
    }

    return true;
}

static void renegotiateFormat(void* userData, uint64_t expirations) {
    Stream* stream = static_cast<Stream*>(userData);
    Instance* instance = stream->instance;
    const spa_pod** params = NULL;

    Logger::logInfo("pipewire", "Renegotiating stream.");

    pw_thread_loop_lock(instance->threadLoop);

    uint8_t paramsBuffer[4096];
    spa_pod_builder builder = SPA_POD_BUILDER_INIT(paramsBuffer, sizeof(paramsBuffer));
    uint32_t numParams;
    if (!buildFormatParams(stream, &builder, params, numParams)) {
        std::cout << "test" << std::endl;
    }
}

static const pw_stream_events streamEvents = {
    .version = PW_VERSION_STREAM_EVENTS,
    .state_changed = onStateChanged,
    .param_changed = onParamChanged,
    .process = onProcess
};

static const pw_core_events coreEvents = {
    .version = PW_VERSION_CORE_EVENTS,
    .done = onCoreDone
};


bool LinuxScreenCapture::pipewireConnectFD(int fd) {
    instance.capture = this;
    instance.fd = fd;
    instance.threadLoop = pw_thread_loop_new("better-replays-pipewire", NULL);
    instance.context = pw_context_new(pw_thread_loop_get_loop(instance.threadLoop), NULL, 0);
    
    if (pw_thread_loop_start(instance.threadLoop) < 0) {
        Logger::logError("pipewire", "Failed to start threaded mainloop.");
        return false;
    }
    Logger::logInfo("pipewire", "Successfully started threaded mainloop -> " + Logger::ptrToStr(instance.threadLoop));

    pw_thread_loop_lock(instance.threadLoop);

    instance.core = pw_context_connect_fd(instance.context, fd, NULL, 0);
    if (!instance.core) {
        Logger::logError("pipewire", "Failed to create core.");
        pw_thread_loop_unlock(instance.threadLoop);
        return false;
    }
    Logger::logInfo("pipewire", "Successfully created core -> " + Logger::ptrToStr(instance.core));

    pw_core_add_listener(instance.core, &instance.coreListener, &coreEvents, (void*)&instance);

    instance.sync_id = pw_core_sync(instance.core, PW_ID_CORE, instance.sync_id);
    pw_thread_loop_wait(instance.threadLoop);

    // registry eventually

    pw_thread_loop_unlock(instance.threadLoop);

    return true;
}

bool LinuxScreenCapture::pipewireConnectStream(int node, StreamInfo& streamInfo) {
    spa_pod_builder builder;
    const spa_pod** params = NULL;
    uint32_t numParams;
    uint8_t paramsBuffer[4096];

    Stream* stream = new Stream();

    stream->instance = &instance;
    stream->cursor.visible = streamInfo.cursorVisible;

    if (streamInfo.framerate) {
        stream->framerate = *streamInfo.framerate;
    }
    if (streamInfo.resolution) {
        stream->resolution = *streamInfo.resolution;
    }

    stream->formatInfo = initFormatInfo();

    pw_thread_loop_lock(instance.threadLoop);

    stream->reneg = pw_loop_add_event(pw_thread_loop_get_loop(instance.threadLoop), renegotiateFormat, stream);
    if (!stream->reneg) {
        Logger::logError("pipewire", "Failed to registered event.");
        return false;
    }
    Logger::logInfo("pipewire", "Successfully registered event -> " + Logger::ptrToStr(stream->reneg));

    stream->stream = pw_stream_new(instance.core, streamInfo.streamName, streamInfo.streamProperties);
    if (!stream->stream) {
        Logger::logError("pipewire", "Failed to create stream.");
        pw_thread_loop_unlock(instance.threadLoop);
        freeStream(stream);
        return false;
    }
    Logger::logInfo("pipewire", "Successfully created stream -> " + Logger::ptrToStr(stream->stream));
    pw_stream_add_listener(stream->stream, &stream->streamListener, &streamEvents, stream);

    builder = SPA_POD_BUILDER_INIT(paramsBuffer, sizeof(paramsBuffer));

    if (!buildFormatParams(stream, &builder, params, numParams)) {
        Logger::logError("pipewire", "Failed to build format parameters.");
        freeStream(stream);
        return false;
    }

    int ret = pw_stream_connect(stream->stream, PW_DIRECTION_INPUT, node, static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS), params, numParams);
    if (ret < 0) {
        Logger::logError("pipewire", "Failed to connect to stream.");
        freeStream(stream);
        return false;
    }
    Logger::logInfo("pipewire", "Successful connection to stream returned -> " + std::to_string(ret));

    pw_thread_loop_unlock(instance.threadLoop);

    delete[] params;

    instance.streams.push_back(stream);

    return true;
}

StreamInfo LinuxScreenCapture::getStreamInfo() {
    pw_properties* properties = pw_properties_new(
        PW_KEY_MEDIA_TYPE, "Video",
        PW_KEY_MEDIA_CATEGORY, "Capture",
        PW_KEY_MEDIA_ROLE, "Screen",
        NULL
    );
    StreamInfo streamInfo = {
        .streamName = "Better Replays",
        .streamProperties = properties,
        .cursorVisible = true
    };

    return streamInfo;
}

std::vector<FormatInfo> LinuxScreenCapture::initFormatInfo() {
    static const struct { uint32_t spa; uint32_t drm; } kFormats[] = {
        { SPA_VIDEO_FORMAT_BGRA, DRM_FORMAT_ARGB8888 },
        { SPA_VIDEO_FORMAT_RGBA, DRM_FORMAT_ABGR8888 },
        { SPA_VIDEO_FORMAT_BGRx, DRM_FORMAT_XRGB8888 },
        { SPA_VIDEO_FORMAT_RGBx, DRM_FORMAT_XBGR8888 },
        { SPA_VIDEO_FORMAT_NV12, DRM_FORMAT_NV12     },
        { SPA_VIDEO_FORMAT_YUY2, DRM_FORMAT_YUYV     },
    };

    std::vector<FormatInfo> formats;

    for (auto& f : kFormats) {
        FormatInfo format;
        format.spaFormat = f.spa;
        format.drmFormat = f.drm;
        formats.push_back(format);
    }

    return formats;
}

void LinuxScreenCapture::freeStream(Stream* stream) {
    if (!stream) return;

    if (stream->stream) {
        pw_stream_disconnect(stream->stream);
        pw_stream_destroy(stream->stream);
        stream->stream = nullptr;
    }
    if (stream->reneg) {
        pw_loop_destroy_source(
            pw_thread_loop_get_loop(stream->instance->threadLoop),
            stream->reneg
        );
        stream->reneg = nullptr;
    }
    delete stream;
}

void LinuxScreenCapture::cleanup() {
    Logger::logInfo("pipewire", "Cleanup called.");
    if (!instance.threadLoop) return;

    pw_thread_loop_lock(instance.threadLoop);

    for (Stream* stream : instance.streams) {
        if (stream->reneg) {
            pw_loop_destroy_source(
                pw_thread_loop_get_loop(instance.threadLoop),
                stream->reneg
            );
            stream->reneg = nullptr;
        }
        if (stream->stream) {
            pw_stream_disconnect(stream->stream);
            pw_stream_destroy(stream->stream);
            stream->stream = nullptr;
        }
        delete stream;
    }
    instance.streams.clear();

    if (instance.core) {
        pw_core_disconnect(instance.core);
        instance.core = nullptr;
    }

    if (instance.context) {
        pw_context_destroy(instance.context);
        instance.context = nullptr;
    }

    pw_thread_loop_unlock(instance.threadLoop);

    pw_thread_loop_stop(instance.threadLoop);
    pw_thread_loop_destroy(instance.threadLoop);
    instance.threadLoop = nullptr;
}

#endif