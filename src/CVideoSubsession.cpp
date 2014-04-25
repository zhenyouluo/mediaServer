#include "CVideoSubsession.h"
#include "CVideoSubSource.h"

CVideoSubsession* CVideoSubsession::createNew(UsageEnvironment &env, CPacketQueue *videoSourceQueue, enum AVCodecID videoEncodeType)
{
  return new CVideoSubsession(env, videoSourceQueue, videoEncodeType);
}
CVideoSubsession::CVideoSubsession(UsageEnvironment &env, CPacketQueue *videoSourceQueue, enum AVCodecID videoEncodeType):OnDemandServerMediaSubsession(env,True),m_pvideoSourceQueue(videoSourceQueue),m_videoEncodeType(videoEncodeType)
{
  //nothing
 }
CVideoSubsession::~CVideoSubsession()
{
  //nothing
}
FramedSource* CVideoSubsession::
createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate)
{
  FramedSource *liveVideoSource = NULL;
  if(m_videoEncodeType == AV_CODEC_ID_H264)
  {
    liveVideoSource = CH264VideoSource::createNew(envir(), m_pvideoSourceQueue);

  }
  else
  {
    liveVideoSource  = CVideoSubSource::createNew(envir(), m_pvideoSourceQueue);
  }

  return createStreamSource(liveVideoSource, estBitrate);
}
RTPSink* CVideoSubsession::
createNewRTPSink(Groupsock* rtpGroupsock,unsigned char rtpPayloadTypeIfDynamic,FramedSource* inputSource)
{
  return createRTPSink(rtpGroupsock, rtpPayloadTypeIfDynamic, inputSource);
}

FramedSource* CVideoSubsession::createStreamSource(FramedSource *framedSubSource, unsigned &estBitrate)
{
  FramedSource * source = NULL;
  estBitrate = 10000; 
  switch(m_videoEncodeType)
  {
    case AV_CODEC_ID_H264:
      {
        //estBitrate = 10000;//maybe incorrect.
        source = H264VideoStreamDiscreteFramer::createNew(envir(),framedSubSource);		
        break;
      }
    case AV_CODEC_ID_MPEG1VIDEO:
    case AV_CODEC_ID_MPEG2VIDEO:
      {
        //estBitrate = 10000;
        source = MPEG1or2VideoStreamDiscreteFramer::createNew(envir(), framedSubSource);
        break;
      }
    case AV_CODEC_ID_MPEG4://for MPEG-4 Video Elementary Stream
      {
        //estBitrate = 10000;
        source = MPEG4VideoStreamDiscreteFramer::createNew(envir(),framedSubSource);
        break;	
      }
    default:
      break;
  }
  return source;
}

RTPSink* CVideoSubsession::
createRTPSink(Groupsock* rtpGroupsock,unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
  RTPSink * sink = NULL;
  switch(m_videoEncodeType)
  {
    case AV_CODEC_ID_H264:
      {
        sink = H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
        break;
      }	
    case AV_CODEC_ID_MPEG1VIDEO:
    case AV_CODEC_ID_MPEG2VIDEO:
      {
        sink = MPEG1or2VideoRTPSink::createNew(envir(), rtpGroupsock);
        break;
      }
    case AV_CODEC_ID_MPEG4://for MPEG-4 Video Elementary Stream
      {
        sink = MPEG4ESVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
        break;
      }	
    default:
      break;
  }
  return sink;
}

