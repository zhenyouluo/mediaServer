#include "CAudioSubsession.h"

CAudioSubsession* CAudioSubsession::createNew(UsageEnvironment &env,CPacketQueue *audioSourceQueue, enum AVCodecID audioType)
{
  return new CAudioSubsession(env, audioSourceQueue, audioType);
}
CAudioSubsession::CAudioSubsession(UsageEnvironment &env, CPacketQueue *audioSourceQueue, enum AVCodecID audioType)
:OnDemandServerMediaSubsession(env, True), faudioSourceQueue(audioSourceQueue), faudioType(audioType)
{
  //nothing
}
CAudioSubsession::~CAudioSubsession()
{
  //nothing......
}
FramedSource* CAudioSubsession::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
{
  FramedSource *source = NULL;
  if(faudioType == AV_CODEC_ID_AAC)
  {
    estBitrate = 128;
    source = CAACAudioSource::createNew(envir(), faudioSourceQueue);
  }
  else
  {
    CAudioSubSource *fitLiveAudioSource = CAudioSubSource::createNew(envir(), faudioSourceQueue,faudioType);
    if (fitLiveAudioSource != NULL)
    {
      source = createStreamSource(fitLiveAudioSource, estBitrate);
    }    
  }
  return source;
}

RTPSink* CAudioSubsession::createNewRTPSink(Groupsock* rtpGroupsock,unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
  return 	createRTPSink(rtpGroupsock, rtpPayloadTypeIfDynamic,inputSource);
}

FramedSource* CAudioSubsession::createStreamSource(FramedSource* FramedSubsource, unsigned& estBitrate)
{
  FramedSource *source = FramedSubsource;
  estBitrate = 128;
  /*  
  switch(faudioType)
  {
    case CODEC_ID_MP3:
      {
        //estBitrate = 128;
        //source = FramedSubsource;
         source = MPEG1or2AudioStreamFramer::createNew(envir(),FramedSubsource);
        break;
      }
    case CODEC_ID_AC3:
      {
       // estBitrate = 192;	
        source = AC3AudioStreamFramer::createNew(envir(),FramedSubsource);
        break;
      }
    case CODEC_ID_MP2:
      {
        //estBitrate = 500;//maybe incorrect
        source = MPEG1or2AudioStreamFramer::createNew(envir(),FramedSubsource);
        break;
      }
    default:
      {
        break;
      }
  }
*/
  return source;
}

RTPSink* CAudioSubsession::createRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
  RTPSink *sink = NULL;
  switch (faudioType)
  {
    case AV_CODEC_ID_MP3:
      {
        sink = MPEG1or2AudioRTPSink::createNew(envir(), rtpGroupsock);
        break;
      }
    case AV_CODEC_ID_MP2:
      {
        sink = MPEG1or2AudioRTPSink::createNew(envir(), rtpGroupsock);
        break;
      }
    case AV_CODEC_ID_AC3:
      {
        AC3AudioStreamFramer* audioSource = (AC3AudioStreamFramer*)inputSource;
        sink = AC3AudioRTPSink::createNew(envir(), rtpGroupsock,rtpPayloadTypeIfDynamic, audioSource->samplingRate());
        break;
      }
    case AV_CODEC_ID_AAC:
      {
        CAACAudioSource *AACAudioSorce = (CAACAudioSource*)inputSource;
        sink = MPEG4GenericRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,AACAudioSorce->samplingFrequency(), "audio", "AAC-hbr",AACAudioSorce->configStr(), AACAudioSorce->numChannels());
        break;
      }
    default:
      break;
  }
  return sink;
}
