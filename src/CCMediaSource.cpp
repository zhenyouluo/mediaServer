
#include "CCMediaSource.h"

unsigned CCMediaSource::delayTime = 100;

CCMediaSource::CCMediaSource(UsageEnvironment &env, CPacketQueue *mediaSourceQueue)
    :FramedSource(env), m_pMediaSourceQueue(mediaSourceQueue)
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
        /*
        m_watchVariable = 0;
        nextTask() = envir().taskScheduler().scheduleDelayedTask(delayTime,
                                                                 (TaskFunc*)delay, this);
        envir().taskScheduler().doEventLoop(&m_watchVariable);
        */

        //try again to get and deliver media data to the sink!
        //doGetNextFrame();
    }

}
void CCMediaSource::delay(void *data)
{
    CCMediaSource *framer = (CCMediaSource *)data;
    framer->setWatchVariable();
}
void CCMediaSource::setWatchVariable()
{
    m_watchVariable = ~0;
}
