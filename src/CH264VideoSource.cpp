#include "CH264VideoSource.h"

CH264VideoSource* CH264VideoSource::createNew(UsageEnvironment &env, CPacketQueue *mediaSourceQueue)
{
  return new CH264VideoSource(env, mediaSourceQueue);
}
CH264VideoSource::CH264VideoSource(UsageEnvironment &env, CPacketQueue *mediaSourceQueue) :CCMediaSource(env, mediaSourceQueue),m_uSecsPerFrame(40000/* 1000000/25 */)
{
  //nothing
}

CH264VideoSource::~CH264VideoSource()
{
  //nothing
}

bool CH264VideoSource::deliverFrame()
{
#ifdef PRINTQUEUEINFO
  cout << "H264 Video Queue size " << fMediaSourceQueue->Size() << endl;
#endif

  LOG(LOG_TYPE_FATAL, "CH264Source Queue Size: %d\n", fMediaSourceQueue->Size());
  //cout << "H264 Video Queue size " << fMediaSourceQueue->Size() << endl;
  AVPacket tempPacket;
  if(fMediaSourceQueue->Pop(&tempPacket))
  {	
    fFrameSize = tempPacket.size;
    unsigned skip = 0;

    unsigned char headers[4];
    if(fFrameSize >= 4)
    {
      memcpy(headers, tempPacket.data, sizeof(headers));
      if(headers[0] == 0 && headers[1] == 0 && ((headers[2] == 0 && headers[3] == 1) || headers[2] == 1))
      {
        if(headers[2] == 1)
        {
          skip = 3;
        }
        else
        {
          skip = 4;
        }
        fFrameSize -= skip;
      }
    }
    if (fFrameSize > fMaxSize)
    {
      //index ++; 
      fNumTruncatedBytes = fFrameSize - fMaxSize;
      LOG(LOG_TYPE_NOTICE, "CH264VideoSource Frame fNumTruncatedBytes: %d Bytes!\n",fNumTruncatedBytes);
      fFrameSize = fMaxSize;
    }
    else
    {
      fNumTruncatedBytes = 0;
    }
    memmove(fTo, tempPacket.data + skip, fFrameSize);
    av_free_packet(&tempPacket);
    if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0)
    {
      // This is the first frame, so use the current time:
      gettimeofday(&fPresentationTime, NULL);
    } 
    else
    {
      // Increment by the play time of the previous frame:
      unsigned uSeconds = fPresentationTime.tv_usec + m_uSecsPerFrame;
      fPresentationTime.tv_sec += uSeconds/1000000;
      fPresentationTime.tv_usec = uSeconds%1000000;
    }  
    fDurationInMicroseconds = m_uSecsPerFrame;
    //FramedSource::afterGetting(this);
    nextTask() = envir().taskScheduler().scheduleDelayedTask(0,(TaskFunc*)FramedSource::afterGetting, this);
    return true;
  }
  return false;
}

