#ifndef  CH264VIDEOSOURCE_H
#define  CH264VIDEOSOURCE_H

#include "liveMedia.hh"
#include "CPacketQueue.h"
#include "CCMediaSource.h"


class CH264VideoSource:public CCMediaSource
{
public:
	static CH264VideoSource* createNew(UsageEnvironment &env, CPacketQueue *mediaSourceQueue);

protected:
	CH264VideoSource(UsageEnvironment &env, CPacketQueue *mediaSourceQueue);
	~CH264VideoSource();

private:
	// redefined virtual functions:
  virtual bool deliverFrame();
	
private:
	unsigned  m_uSecsPerFrame;
};
#endif
