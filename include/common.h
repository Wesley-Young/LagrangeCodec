//
// Created by Wenxuan Lin on 2025-02-23.
//

#ifndef COMMON_H
#define COMMON_H

#include <cstdint>

extern "C" {
#include <libavformat/avio.h>
#include <libavformat/avformat.h>
}

typedef void (cb_codec)(void* userdata, const uint8_t* p, int len);

#ifdef _WIN32
#define __dllexport __declspec(dllexport)
#else
#define __dllexport
#endif

#define EXPORT extern "C" __dllexport

// REMEMBER TO FREE THE BUFFER AFTER USE BY av_free(format_context->pb->buffer);
inline int create_format_context(uint8_t* data, int data_len, AVFormatContext** format_context) {
    AVIOContext* avio_ctx = nullptr; // Create a custom I/O context with the raw audio data buffer
    uint8_t* avio_buffer = nullptr;

    avio_buffer = static_cast<uint8_t*>(av_malloc(data_len)); // Allocate buffer for AVIOContext
    if (!avio_buffer) {
        fprintf(stderr, "ERROR: failed to allocate memory for AVIOContext\n");
        return -1;
    }
    memcpy(avio_buffer, data, data_len);

    // Allocate the AVIOContext with the custom buffer
    avio_ctx = avio_alloc_context(avio_buffer, data_len, 0, nullptr, nullptr, nullptr, nullptr);
    if (!avio_ctx) {
        fprintf(stderr, "ERROR: failed to create AVIOContext\n");
        return -1;
    }

    // Create format context and set the I/O context
    *format_context = avformat_alloc_context();
    (*format_context)->pb = avio_ctx;

    return 0;
}

#endif //COMMON_H
