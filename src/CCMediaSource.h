
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

    void setWatchVariable();
private:
    virtual void doGetNextFrame();
    virtual bool deliverFrame() = 0;
    static void delay(void *data);
    static unsigned delayTime;
    char m_watchVariable;

protected:
    CPacketQueue *m_pMediaSourceQueue;
};
#endif
