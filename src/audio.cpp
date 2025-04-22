//
// Created by Wenxuan Lin on 2025-02-23.
//

#include "audio.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}

int audio_to_pcm(uint8_t* audio_data, int data_len, cb_codec callback, void* userdata) {
    AVFormatContext* format_context = nullptr;
    int ret;
    if (create_format_context(audio_data, data_len, &format_context) < 0) {
        fprintf(stderr, "ERROR: failed to create format context\n");
        return -1;
    }

    if (avformat_open_input(&format_context, nullptr, nullptr, nullptr) < 0) {
        av_free(format_context->pb->buffer);
        return -1;
    }

    ret = avformat_find_stream_info(format_context, nullptr);
    if (ret < 0) {
        fprintf(stderr, "ERROR: failed to stream info \n");
        return -1;
    }

    printf("DEBUG: number of streams found: %d\n", format_context->nb_streams);
    int stream_index = av_find_best_stream(format_context, AVMEDIA_TYPE_AUDIO,-1, -1, nullptr, 0);
    if (stream_index < 0) {
        fprintf(stderr, "ERROR: no audio stream found\n");
        return -1;
    }

    const AVStream *stream = format_context->streams[stream_index];

    const AVCodec *decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder) {
        fprintf(stderr, "ERROR: no decoder found\n");
        return -1;
    }

    AVCodecContext *decoder_ctx = avcodec_alloc_context3(decoder);

    avcodec_parameters_to_context(decoder_ctx, stream->codecpar);

    ret = avcodec_open2(decoder_ctx, decoder, nullptr);
    if (ret < 0) {
        fprintf(stderr, "ERROR: failed to open the decoder\n");
        return -1;
    }

    printf(
        "DEBUG: Setting up decoder - sample format: %s, sample rate: %d Hz, "
        "channels: %d \n",
        av_get_sample_fmt_name(static_cast<AVSampleFormat>(stream->codecpar->format)),
        stream->codecpar->sample_rate, stream->codecpar->channels);

    // Step 2: Decode audio
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    SwrContext *swr_context = swr_alloc_set_opts(nullptr,
    AV_CH_LAYOUT_MONO, AV_SAMPLE_FMT_S16,24000,
    stream->codecpar->channel_layout,static_cast<AVSampleFormat>(stream->codecpar->format), stream->codecpar->sample_rate,
    0, nullptr);

    while (av_read_frame(format_context, packet) == 0) {
        if (packet->stream_index != stream_index) continue;

        ret = avcodec_send_packet(decoder_ctx, packet);
        if (ret < 0 && (ret != AVERROR(EAGAIN))) {
            fprintf(stderr, "ERROR: failed to decode a frame\n");
        }
        while ((ret = avcodec_receive_frame(decoder_ctx, frame)) == 0) {
            AVFrame *resampled_frame = av_frame_alloc();
            resampled_frame->sample_rate = 24000;
            resampled_frame->channel_layout = AV_CH_LAYOUT_MONO;
            resampled_frame->channels = 1;
            resampled_frame->format = AV_SAMPLE_FMT_S16;

            ret = swr_convert_frame(swr_context, resampled_frame, frame);

            callback(userdata, resampled_frame->data[0], resampled_frame->nb_samples * resampled_frame->channels * 2);
            av_frame_unref(frame);
            av_frame_free(&resampled_frame);
        }
    }

    swr_free(&swr_context);
    avcodec_close(decoder_ctx);
    avformat_close_input(&format_context);

    return 0;
}

