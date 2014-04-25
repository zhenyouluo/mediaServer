#ifndef  CVIDEOSUBSOURCE_H
#define  CVIDEOSUBSOURCE_H

#include "liveMedia.hh"
//#include "definition.h"
#include "CPacketQueue.h"
#include "CCMediaSource.h"


class CVideoSubSource:public CCMediaSource
{
public:
	static CVideoSubSource* createNew(UsageEnvironment &env, CPacketQueue *mediaSourceQueue);

protected:
	CVideoSubSource(UsageEnvironment &env, CPacketQueue *mediaSourceQueue);
	~CVideoSubSource();

private:
	// redefined virtual functions:
  virtual bool deliverFrame();
private:
  unsigned m_uSecsPerFrame;
 };
#endif
