#ifndef CAACAUDIOSOURCE_H
#define CAACAUDIOSOURCE_H

#include "liveMedia.hh"
#include "CPacketQueue.h"
#include "CCMediaSource.h"

class CAACAudioSource:public CCMediaSource
{
  public:
    static CAACAudioSource *createNew(UsageEnvironment &env, CPacketQueue *dataPacketQueue);
    unsigned samplingFrequency()const
    {
      return fSamplingFrequency;
    }
    unsigned numChannels()const
    {
      return fNumChannels;
    }
    char const* configStr()const
    {
      return fConfigStr;
    }
  protected:
    CAACAudioSource(UsageEnvironment &env, CPacketQueue *dataPacketQueue, u_int8_t sampling_frequency_index, u_int8_t channelConfiguration, u_int8_t profile);
    ~CAACAudioSource();

  private:
    // redefined virtual functions:
    virtual bool deliverFrame();

  private:
    unsigned fSamplingFrequency;
    unsigned fNumChannels;
    unsigned m_uSecsPerFrame;
    char fConfigStr[5];
};

#endif
