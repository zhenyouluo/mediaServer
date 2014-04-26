#ifndef CAACAUDIOSUBSOURCE_H
#define CAACAUDIOSUBSOURCE_H

#include "liveMedia.hh"
#include "CPacketQueue.h"
#include "CCMediaSource.h"

class CAACAudioSubSource:public CCMediaSource
{
  public:
    static CAACAudioSubSource *createNew(UsageEnvironment &env, CPacketQueue *dataPacketQueue);
    unsigned samplingFrequency()const
    {
      return m_samplingFrequency;
    }
    unsigned numChannels()const
    {
      return m_numChannels;
    }
    char const* configStr()const
    {
      return m_configStr;
    }
  protected:
    CAACAudioSubSource(UsageEnvironment &env, CPacketQueue *dataPacketQueue,
                    u_int8_t sampling_frequency_index, u_int8_t
                    channelConfiguration, u_int8_t profile);
    ~CAACAudioSubSource();

  private:
    // redefined virtual functions:
    virtual bool deliverFrame();

  private:
    unsigned m_samplingFrequency;
    unsigned m_numChannels;
    unsigned m_uSecsPerFrame;
    char m_configStr[5];
};

#endif
