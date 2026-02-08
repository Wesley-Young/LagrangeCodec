//
// Created by Wenxuan Lin on 2025-02-23.
//

#ifndef COMMON_H
#define COMMON_H

#include <cstdint>

typedef void (cb_codec)(void* userdata, const uint8_t* p, int len);

#ifdef _WIN32
#  if defined(LAGRANGECODEC_SHARED_BUILD)
#    define LAGRANGECODEC_API __declspec(dllexport)
#  elif defined(LAGRANGECODEC_SHARED)
#    define LAGRANGECODEC_API __declspec(dllimport)
#  else
#    define LAGRANGECODEC_API
#  endif
#else
#  define LAGRANGECODEC_API
#endif

#define EXPORT extern "C" LAGRANGECODEC_API

#endif //COMMON_H
