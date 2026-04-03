//
// Created by Wenxuan Lin on 2025-02-23.
//



#ifndef AUDIO_CPP_H
#define AUDIO_CPP_H

#include "common.h"

constexpr int SILKV3_SAMPLE_RATE = 24000;

EXPORT int audio_to_pcm(uint8_t* audio_data, int data_len, cb_codec callback, void *userdata);

#endif //AUDIO_CPP_H