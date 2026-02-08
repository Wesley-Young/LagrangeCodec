 //
// Created by Wenxuan Lin on 2025-02-23.
//

#ifndef SILK_H
#define SILK_H

#include "common.h"

#define MAX_INPUT_FRAMES        5
#define MAX_FRAME_LENGTH        480
#define FRAME_LENGTH_MS         20
#define MAX_API_FS_KHZ          48
#define MAX_LBRR_DELAY          2
#define MAX_BYTES_PER_FRAME     250 // Equals peak bitrate of 100 kbps

#ifdef _SYSTEM_IS_BIG_ENDIAN
inline void swap_endian(SKP_int16 vec[], SKP_int len) {
    SKP_int16 tmp;

    for (SKP_int i = 0; i < len; i++) {
        tmp = vec[i];
        SKP_uint8 *p1 = reinterpret_cast<unsigned char *>(&vec[i]);
        SKP_uint8 *p2 = reinterpret_cast<unsigned char *>(&tmp);
        p1[0] = p2[1]; p1[1] = p2[0];
    }
}
#endif

EXPORT int silk_decode(uint8_t* silk_data, int len, cb_codec callback, void* userdata);

EXPORT int silk_encode(uint8_t* pcm_data, int len, cb_codec callback, void* userdata);

#endif //SILK_H
