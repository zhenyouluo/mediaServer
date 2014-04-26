#ifndef  CH264VIDEOSubSOURCE_H
#define  CH264VIDEOSubSOURCE_H

#include "liveMedia.hh"
#include "CPacketQueue.h"
#include "CCMediaSource.h"


class CH264VideoSubSource:public CCMediaSource
{
public:
    static CH264VideoSubSource* createNew(UsageEnvironment &env, CPacketQueue *mediaSourceQueue, double fps = 25);

protected:
    CH264VideoSubSource(UsageEnvironment &env, CPacketQueue *mediaSourceQueue, double fps);
    ~CH264VideoSubSource();

private:
	// redefined virtual functions:
  virtual bool deliverFrame();
	
private:
    double  m_uSecsPerFrame;
};
#endif
