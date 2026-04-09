#include <ffmpeg_encoder.hpp>

// better replays
#include <logger.hpp>

#include <iostream>

static AVPixelFormat pixelFormatToAVPixelFormat(PixelFormat pixelFormat) {
    switch (pixelFormat) {
        case PixelFormat::NONE: return AV_PIX_FMT_NONE;
        case PixelFormat::BGRA: return AV_PIX_FMT_BGRA;
        case PixelFormat::RGBA: return AV_PIX_FMT_RGBA;
        case PixelFormat::BGRx: return AV_PIX_FMT_BGR0;
        case PixelFormat::RGBx: return AV_PIX_FMT_RGB0;
        case PixelFormat::NV12: return AV_PIX_FMT_NV12;
        case PixelFormat::YUY2: return AV_PIX_FMT_YUYV422;
        default: return AV_PIX_FMT_NONE;
    }
}

static int getScaleAlgorithm(std::string scaling) {
    if (scaling == "fast_bilinear") {
        return SWS_FAST_BILINEAR;
    } else if (scaling == "bilinear") {
        return SWS_BILINEAR;
    } else if (scaling == "bicubic") {
        return SWS_BICUBIC;
    } else if (scaling == "x") {
        return SWS_X;
    } else if (scaling == "point") {
        return SWS_POINT;
    } else if (scaling == "area") {
        return SWS_AREA;
    } else if (scaling == "bicublin") {
        return SWS_BICUBLIN;
    } else if (scaling == "gauss") {
        return SWS_GAUSS;
    } else if (scaling == "sinc") {
        return SWS_SINC;
    } else if (scaling == "lanczos") {
        return SWS_LANCZOS;
    } else if (scaling == "spline") {
        return SWS_SPLINE;
    } else {
        return SWS_FAST_BILINEAR;
    }
}

static const AVCodec* findEncoder() {
    static const char* hardwareEncoders[] = {
        "h264_nvenc",
        "h264_vaapi",
        "h264_qsv",
        "h264_amf",
        nullptr
    };
    int numEncoders = 5;

    for (int i = 0; i < numEncoders; ++i) {
        const AVCodec* codec = avcodec_find_encoder_by_name(hardwareEncoders[i]);
        if (codec) {
            Logger::logInfo("ffmpeg", "Using hardware encode: " + std::string(hardwareEncoders[i]));
            return codec;
        }
    }

    Logger::logInfo("ffmpeg", "Falling back to software encoder: libx264");
    return avcodec_find_encoder(AV_CODEC_ID_H264);
}

FFmpegEncoder::~FFmpegEncoder() {
    flush();
    if (codecContext) {
        avcodec_free_context(&codecContext);
    }
    if (avFrame) {
        av_frame_free(&avFrame);
    }
    if (avPacket) {
        av_packet_free(&avPacket);
    }
    if (swsContext) {
        sws_freeContext(swsContext);
    }
}

bool FFmpegEncoder::init(int srcWidth, int srcHeight, PixelFormat srcFormat, int dstWidth, int dstHeight, int fps) {
    const AVCodec* codec = findEncoder();
    if (!codec) {
        Logger::logError("ffmpeg", "Failed to find any encoder.");
        return false;
    }

    codecContext = avcodec_alloc_context3(codec);
    codecContext->width     = dstWidth;
    codecContext->height    = dstHeight;
    codecContext->time_base = {1, 1000};
    codecContext->framerate = {fps, 1};
    codecContext->gop_size  = 10;
    codecContext->flags    |= AV_CODEC_FLAG_GLOBAL_HEADER;
    codecContext->color_range    = AVCOL_RANGE_MPEG;
    codecContext->colorspace     = AVCOL_SPC_BT709;
    codecContext->color_trc      = AVCOL_TRC_BT709;
    codecContext->color_primaries = AVCOL_PRI_BT709;

    codecName = codec->name;

    isHardware = (codecName == "h264_vaapi" || codecName == "h264_nvenc" || codecName == "h264_qsv" || codecName == "h264_amf");

    if (codecName == "h264_vaapi") {
        if (!initVaapi(dstWidth, dstHeight)) return false;
    } else if (codecName == "h264_nvenc") {
        initNvenc();
    } else {
        initSoftware();
    }

    if (avcodec_open2(codecContext, codec, NULL) < 0) {
        Logger::logError("ffmpeg", "Failed to open codec.");
        return false;
    }

    // vaapi uploads frames differently, software path uses swscale
    if (codecName != "h264_vaapi") {
        swsContext = sws_getContext(
            srcWidth, srcHeight, pixelFormatToAVPixelFormat(srcFormat),
            dstWidth, dstHeight, AV_PIX_FMT_YUV420P,
            getScaleAlgorithm(settings->ffmpeg_encoder.scaling), NULL, NULL, NULL
        );
        sws_setColorspaceDetails(
            swsContext,
            sws_getCoefficients(SWS_CS_ITU709), 0,
            sws_getCoefficients(SWS_CS_ITU709), 1,
            0, 1 << 16, 1 << 16
        );
    }

    avFrame = av_frame_alloc();
    avFrame->format = codecContext->pix_fmt;
    avFrame->width  = dstWidth;
    avFrame->height = dstHeight;
    av_frame_get_buffer(avFrame, 0);

    avPacket = av_packet_alloc();

    codecParams.width  = dstWidth;
    codecParams.height = dstHeight;
    codecParams.fps    = fps;
    if (codecContext->extradata && codecContext->extradata_size > 0) {
        codecParams.extraData = std::vector<uint8_t>(
            codecContext->extradata,
            codecContext->extradata + codecContext->extradata_size
        );
    }

    return true;
}
bool FFmpegEncoder::encode(const Frame& frame) {
    const uint8_t* srcData[] = { frame.data.data() };
    int srcLinesize[]        = { frame.stride };
    sws_scale(swsContext, srcData, srcLinesize, 0, frame.height, avFrame->data, avFrame->linesize);

    avFrame->pts = av_rescale_q(frame.timestampMS, {1, 1000}, codecContext->time_base);

    if (codecName == "h264_vaapi") {
        AVFrame* hwFrame = av_frame_alloc();
        if (av_hwframe_get_buffer(codecContext->hw_frames_ctx, hwFrame, 0) < 0) {
            Logger::logError("ffmpeg", "Failed to get hw frame buffer.");
            av_frame_free(&hwFrame);
            return false;
        }
        if (av_hwframe_transfer_data(hwFrame, avFrame, 0) < 0) {
            Logger::logError("ffmpeg", "Failed to transfer frame to hw.");
            av_frame_free(&hwFrame);
            return false;
        }
        hwFrame->pts = avFrame->pts;
        avcodec_send_frame(codecContext, hwFrame);
        av_frame_free(&hwFrame);
    } else {
        avcodec_send_frame(codecContext, avFrame);
    }

    int ret = 0;
    while (ret >= 0) {
        ret = avcodec_receive_packet(codecContext, avPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;

        EncodedPacket packet;
        packet.data.insert(packet.data.end(), avPacket->data, avPacket->data + avPacket->size);
        packet.presentationTimestamp = av_rescale_q(avPacket->pts, codecContext->time_base, {1, 1000});
        packet.decodeTimestamp       = av_rescale_q(avPacket->dts, codecContext->time_base, {1, 1000});
        packet.isKeyframe            = (avPacket->flags & AV_PKT_FLAG_KEY);

        if (onPacket) onPacket(packet);
        av_packet_unref(avPacket);
    }

    return true;
}
void FFmpegEncoder::flush() {
    avcodec_send_frame(codecContext, NULL);
}

void FFmpegEncoder::initSoftware() {
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    codecContext->max_b_frames = 2;
    codecContext->has_b_frames = 1;
    av_opt_set(codecContext->priv_data, "crf", settings->ffmpeg_encoder.crf.c_str(), 0);
    av_opt_set(codecContext->priv_data, "preset", settings->ffmpeg_encoder.preset.c_str(), 0);
    av_opt_set(codecContext->priv_data, "tune", settings->ffmpeg_encoder.tune.c_str(), 0);
}
void FFmpegEncoder::initNvenc() {
    codecContext->pix_fmt      = AV_PIX_FMT_YUV420P;
    codecContext->max_b_frames = 0;
    codecContext->has_b_frames = 0;
    av_opt_set(codecContext->priv_data, "preset", "p4",  0);
    av_opt_set(codecContext->priv_data, "tune",   "hq",  0);
    av_opt_set(codecContext->priv_data, "rc",     "vbr", 0);
    av_opt_set(codecContext->priv_data, "cq",     settings->ffmpeg_encoder.crf.c_str(), 0);
}
bool FFmpegEncoder::initVaapi(int width, int height) {
    int ret = av_hwdevice_ctx_create(&hwDeviceCtx, AV_HWDEVICE_TYPE_VAAPI, nullptr, nullptr, 0);
    if (ret < 0) {
        Logger::logError("ffmpeg", "Failed to create VAAPI device.");
        return false;
    }

    AVBufferRef* hwFramesRef = av_hwframe_ctx_alloc(hwDeviceCtx);
    AVHWFramesContext* framesCtx = (AVHWFramesContext*)hwFramesRef->data;
    framesCtx->format    = AV_PIX_FMT_VAAPI;
    framesCtx->sw_format = AV_PIX_FMT_NV12;
    framesCtx->width     = width;
    framesCtx->height    = height;
    framesCtx->initial_pool_size = 20;
    av_hwframe_ctx_init(hwFramesRef);

    codecContext->pix_fmt    = AV_PIX_FMT_VAAPI;
    codecContext->hw_device_ctx = av_buffer_ref(hwDeviceCtx);
    codecContext->hw_frames_ctx = hwFramesRef;

    av_opt_set(codecContext->priv_data, "rc_mode", "CQP", 0);
    av_opt_set(codecContext->priv_data, "qp", settings->ffmpeg_encoder.crf.c_str(), 0);

    return true;
}