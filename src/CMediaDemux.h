#ifndef CMEDIADEMUX_H
#define CMEDIADEMUX_H

#include "ffmpeg.h"

class CMediaDemux
{
public:
  CMediaDemux(const char* fileName);
  ~CMediaDemux();

  int readPacket(AVPacket &pkt);

  AVCodecContext* videoCodecCnt() const
  {
    return m_pvideoCodecContext;
  }
  AVCodecContext* audioCodecCnt()const
  {
    return m_paudioCodecContext;
  }
  AVFormatContext* formatContext()const
  {
    return m_pFormatCtx;
  }

  int videoStreamIndex()const
  {
    return m_videoStreamIndex;
  }
  int audioStreamIndex()const
  {
    return m_audioStreamIndex;
  }

  enum AVCodecID audioCodeId()const
  {
    return m_audioCodeId;
  }
  enum AVCodecID videoCodeId()const
  {
    return m_videoCodeId;
  }
private:
  void setUp();
  void unSetUp();

private:
  AVFormatContext *m_pFormatCtx;
  AVCodec *m_pvideoCodec;
  AVCodecContext *m_pvideoCodecContext;
  int m_videoStreamIndex;
  AVCodec *m_paudioCodec;
  AVCodecContext *m_paudioCodecContext;
  int m_audioStreamIndex;
  AVPacket AvPacket;
  AVFrame *m_pFrame;
  enum AVCodecID m_audioCodeId;
  enum AVCodecID m_videoCodeId;
  AVRational rate;
  int64_t best_effort_timestamp;

  const char *m_pSrcFile;

  bool m_notSetUp;
};

#endif

