
#include "CCMediaSource.h"

unsigned CCMediaSource::delayTime = 10;

CCMediaSource::CCMediaSource(UsageEnvironment &env, CPacketQueue *mediaSourceQueue)
:FramedSource(env), fMediaSourceQueue(mediaSourceQueue)
{
  //nothing
  //
}
CCMediaSource::~CCMediaSource()
{
  //nothing
}
void CCMediaSource::doGetNextFrame()
{
  if(!deliverFrame())
  {
    fwatchVariable = 0;
    nextTask() =  envir().taskScheduler().scheduleDelayedTask(delayTime, (TaskFunc*)delay, this);
    envir().taskScheduler().doEventLoop(&fwatchVariable);
    //try again to get and deliver media data to the sink!
    doGetNextFrame();
  }
}
bool CCMediaSource::deliverFrame()
{
  return false;
}
void CCMediaSource::delay(void *data)
{
  CCMediaSource *framer = (CCMediaSource *)data;
  framer->fwatchVariable = ~0;
}
