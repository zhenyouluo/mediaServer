#ifndef  FFMPEG_H
#define  FFMPEG_H

#if defined(__WIN32__)||defined(_WIN32)
#define inline _inline
#endif

#ifndef INT64_C
#define INT64_C
#define UINT64_C
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>

#ifdef __cplusplus
}
#endif

#if defined(__WIN32__)||defined(_WIN32)
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "postproc.lib")
#pragma comment(lib, "swresample.lib")
#endif

#include <iostream>
#include <queue>
using namespace std;

#endif



