#ifndef  CAUDIOSUBSOURCE_H
#define  CAUDIOSUBSOURCE_H

#include "liveMedia.hh"
#include "CPacketQueue.h"
#include "CCMediaSource.h"

class CAudioSubSource:public CCMediaSource
{
public:
  static CAudioSubSource* createNew(UsageEnvironment &env, CPacketQueue *mediaSourceQueue, enum AVCodecID encodeID);
  unsigned getSamples()const
  {
    return m_samplesFreq;
  }
protected:
  CAudioSubSource(UsageEnvironment &env, CPacketQueue *mediaSourceQueue, unsigned sampleFreq, unsigned secsPerFrame);
  ~CAudioSubSource();

private:
  virtual bool deliverFrame();
  unsigned m_samplesFreq;
  unsigned m_uSecsPerFrame;
};

#endif
