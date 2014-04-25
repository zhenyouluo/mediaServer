
#ifndef CCMEDIASOURCE_H
#define CCMEDIASOURCE_H

#include "liveMedia.hh"
#include "CPacketQueue.h"

#include"CLog.h"

//#define PRINTQUEUEINFO 1

class CCMediaSource:public FramedSource
{
  public:
    CCMediaSource(UsageEnvironment &env, CPacketQueue *mediaSourceQueue);
    virtual ~CCMediaSource();
    char fwatchVariable;
  private:
    virtual void doGetNextFrame();
    virtual bool deliverFrame();    
    static void delay(void *data);
    static unsigned delayTime;

  protected:
    CPacketQueue *fMediaSourceQueue;

};
#endif
