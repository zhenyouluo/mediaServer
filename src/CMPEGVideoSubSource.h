#ifndef  CMPEGVIDEOSUBSOURCE_H
#define  CMPEGVIDEOSUBSOURCE_H

#include "liveMedia.hh"
#include "CPacketQueue.h"
#include "CCMediaSource.h"


class CMPEGVideoSubSource:public CCMediaSource
{
public:
    static CMPEGVideoSubSource* createNew(UsageEnvironment &env, CPacketQueue *mediaSourceQueue,
                                      double fps=25);

protected:
    CMPEGVideoSubSource(UsageEnvironment &env, CPacketQueue *mediaSourceQueue, double fps);
    ~CMPEGVideoSubSource();

private:
	// redefined virtual functions:
  virtual bool deliverFrame();
private:
  double m_uSecsPerFrame;
 };
#endif
