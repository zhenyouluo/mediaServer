#include "CMPEGVideoSubSource.h"

CMPEGVideoSubSource* CMPEGVideoSubSource::createNew(UsageEnvironment &env, CPacketQueue *mediaSourceQueue,
                                            double fps)
{
  return new CMPEGVideoSubSource(env, mediaSourceQueue, fps);
}
CMPEGVideoSubSource::CMPEGVideoSubSource(UsageEnvironment &env, CPacketQueue *mediaSourceQueue, double fps)
:CCMediaSource(env, mediaSourceQueue),m_uSecsPerFrame( 1000000.0/fps)
{
  //nothing
}
CMPEGVideoSubSource::~CMPEGVideoSubSource()
{
  //nothing
}
bool CMPEGVideoSubSource::deliverFrame()
{
  AVPacket tempPacket;
  if(m_pMediaSourceQueue->Pop(&tempPacket))
  {	
    fFrameSize = tempPacket.size;
    if (fFrameSize > fMaxSize)
    {
      fNumTruncatedBytes = fFrameSize - fMaxSize;
      LOG(LOG_TYPE_NOTICE, "CVideoSubSource Frame fNumTruncatedBytes : %d Bytes!\n",
          fNumTruncatedBytes);
      fFrameSize = fMaxSize;
    }
    else
    {
      fNumTruncatedBytes = 0;
    }
    memmove(fTo, tempPacket.data , fFrameSize);
    av_free_packet(&tempPacket);

    if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0)
    {
      // This is the first frame, so use the current time:
      gettimeofday(&fPresentationTime, NULL);
    } 
    else
    {
      // Increment by the play time of the previous frame:
      unsigned uSeconds = (unsigned)(fPresentationTime.tv_usec + m_uSecsPerFrame);
      fPresentationTime.tv_sec += uSeconds/1000000;
      fPresentationTime.tv_usec = uSeconds%1000000;
    }  
    fDurationInMicroseconds = m_uSecsPerFrame;

    nextTask() = envir().taskScheduler().scheduleDelayedTask(0,	(TaskFunc*)FramedSource::afterGetting, this);
    return true;
  }
  return false;
}

