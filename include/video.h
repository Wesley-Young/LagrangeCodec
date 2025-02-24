//
// Created by Wenxuan Lin on 2025-02-23.
//

#ifndef VIDEO_H
#define VIDEO_H

#include "common.h"

#include <cstdint>

struct VideoInfo {
    int width;
    int height;
    int64_t duration;
};

EXPORT int video_first_frame(uint8_t* video_data, int data_len, uint8_t*& out, int& out_len);

EXPORT int video_get_size(uint8_t* video_data, int data_len, VideoInfo& info);

#endif //VIDEO_H
