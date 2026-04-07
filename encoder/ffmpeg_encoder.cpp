#include <ffmpeg_encoder.hpp>

// better replays
#include <logger.hpp>

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

bool FFmpegEncoder::init(int width, int height, int fps) {
    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        Logger::logError("ffmpeg", "Failed to find codec.");
        return false;
    }

    codecContext = avcodec_alloc_context3(codec);
    codecContext->width = width;
    codecContext->height = height;
    codecContext->time_base = {1, fps};
    codecContext->framerate = {fps, 1};
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    codecContext->gop_size = 10; 
    codecContext->max_b_frames = 2;
    codecContext->has_b_frames = 1;
    codecContext->color_range = AVCOL_RANGE_JPEG;
    codecContext->colorspace = AVCOL_SPC_BT709;
    codecContext->color_trc = AVCOL_TRC_BT709;
    codecContext->color_primaries = AVCOL_PRI_BT709;

    av_opt_set(codecContext->priv_data, "crf", "10", 0);
    av_opt_set(codecContext->priv_data, "preset", "veryfast", 0);
    av_opt_set(codecContext->priv_data, "tune", "film", 0);

    if (avcodec_open2(codecContext, codec, NULL) < 0) return false;

    avFrame = av_frame_alloc();
    avFrame->format = codecContext->pix_fmt;
    avFrame->width = width;
    avFrame->height = height;
    av_frame_get_buffer(avFrame, 0);

    avPacket = av_packet_alloc();

    swsContext = sws_getContext(
        width, height, AV_PIX_FMT_BGRA,
        width, height, AV_PIX_FMT_YUV420P,
        SWS_FAST_BILINEAR, NULL, NULL, NULL
    );

    return true;
}
bool FFmpegEncoder::encode(const Frame& frame) {
    const uint8_t* srcData[] = { frame.data.data() };
    int srcLinesize[] = { frame.stride };
    
    sws_scale(
        swsContext, 
        srcData, 
        srcLinesize, 
        0, 
        frame.height, 
        avFrame->data, 
        avFrame->linesize
    );

    avFrame->pts = frameCounter++;

    int ret = avcodec_send_frame(codecContext, avFrame);
    if (ret < 0) {
        Logger::logError("ffmpeg", "Failed to send frame.");
        return false;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(codecContext, avPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
        
        EncodedPacket packet;
        packet.data.insert(packet.data.end(), avPacket->data, avPacket->data + avPacket->size);
        packet.presentationTimestamp = av_rescale_q(avPacket->pts, codecContext->time_base, {1, 1000});
        packet.decodeTimestamp = av_rescale_q(avPacket->dts, codecContext->time_base, {1, 1000});
        packet.isKeyframe = (avPacket->flags & AV_PKT_FLAG_KEY);

        if (onPacket) onPacket(packet);
        
        av_packet_unref(avPacket);
    }
    return true;
}
void FFmpegEncoder::flush() {
    avcodec_send_frame(codecContext, NULL);
}