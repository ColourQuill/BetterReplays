#include <ffmpeg_muxer.hpp>

// better replays
#include <logger.hpp>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

bool FFmpegMuxer::save(const std::string& filename, const std::vector<EncodedPacket>& packets, int width, int height, int fps) {
    if (packets.empty()) {
        return false;
    }

    char errbuf[256];

    int ret = avformat_alloc_output_context2(&formatContext, nullptr, nullptr, filename.c_str());
    if (ret < 0) {
        av_strerror(ret, errbuf, sizeof(errbuf));
        Logger::logError("Muxer", "Context alloc failed -> " + std::string(errbuf));
        return false;
    }

    videoStream = avformat_new_stream(formatContext, nullptr);
    if (!videoStream) {
        avformat_free_context(formatContext);
        return false;
    }

    videoStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    videoStream->codecpar->codec_id = AV_CODEC_ID_H264;
    videoStream->codecpar->width = width;
    videoStream->codecpar->height = height;
    videoStream->codecpar->format = AV_PIX_FMT_YUV420P;
    videoStream->time_base = {1, 1000}; 
    videoStream->avg_frame_rate = {fps, 1};

    if (!(formatContext->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&formatContext->pb, filename.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_strerror(ret, errbuf, sizeof(errbuf));
            Logger::logError("Muxer", "Could not open file -> " + std::string(errbuf));
            avformat_free_context(formatContext);
            return false;
        }
    }

    ret = avformat_write_header(formatContext, nullptr);
    if (ret < 0) {
        av_strerror(ret, errbuf, sizeof(errbuf));
        Logger::logError("Muxer", "Header write failed -> " + std::string(errbuf));
        if (formatContext->pb) {
            avio_closep(&formatContext->pb);
        }
        avformat_free_context(formatContext);
        return false;
    }

    for (const auto& packet : packets) {
        AVPacket* avPacket = av_packet_alloc();
        
        avPacket->data = const_cast<uint8_t*>(packet.data.data());
        avPacket->size = (int)packet.data.size();
        
        avPacket->pts = packet.presentationTimestamp;
        avPacket->dts = packet.decodeTimestamp;
        avPacket->duration = 1000 / fps;
        
        if (packet.isKeyframe) {
            avPacket->flags |= AV_PKT_FLAG_KEY;
        }

        avPacket->stream_index = videoStream->index;

        av_packet_rescale_ts(avPacket, {1, 1000}, videoStream->time_base);

        ret = av_interleaved_write_frame(formatContext, avPacket);
        
        avPacket->data = nullptr; 
        av_packet_free(&avPacket);

        if (ret < 0) {
            break;
        }
    }

    av_write_trailer(formatContext);

    if (!(formatContext->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&formatContext->pb);
    }
    
    avformat_free_context(formatContext);
    formatContext = nullptr;

    return true;
}