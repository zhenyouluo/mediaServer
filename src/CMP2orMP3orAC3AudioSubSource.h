#ifndef  CMP2orMP3orAC3AUDIOSUBSOURCE_H
#define   CMP2orMP3orAC3AUDIOSUBSOURCE_H

#include "liveMedia.hh"
#include "CPacketQueue.h"
#include "CCMediaSource.h"

class CMP2orMP3orAC3AudioSubSource:public CCMediaSource
{
public:
  static CMP2orMP3orAC3AudioSubSource* createNew(UsageEnvironment &env, CPacketQueue *mediaSourceQueue,
                                                 enum AVCodecID encodeID);
  unsigned getSamples()const
  {
    return m_samplesFreq;
  }
protected:
  CMP2orMP3orAC3AudioSubSource(UsageEnvironment &env, CPacketQueue *mediaSourceQueue,
                               unsigned sampleFreq, unsigned secsPerFrame);
  ~CMP2orMP3orAC3AudioSubSource();

private:
  virtual bool deliverFrame();
  unsigned m_samplesFreq;
  unsigned m_uSecsPerFrame;
};

#endif
