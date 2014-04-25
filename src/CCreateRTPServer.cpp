#include "CCreateRTPServer.h"
#include "CH264VideoSource.h"
#include "CAudioSubSource.h"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
#include "CVideoSubSource.h"
#include "CAACAudioSource.h"

CCreateRTPServer::CCreateRTPServer(CMediaState *ptrState, CPacketQueue *paudioSourceQueue, enum AVCodecID audioEncodeID, CPacketQueue *pvideoSourceQueue, enum AVCodecID videoEncodeID,unsigned short rtpPortNum, unsigned short rtspPortNum, unsigned char ttl):m_pMediaState(ptrState), m_pAudioSourceQueue(paudioSourceQueue),m_AudioEncodeID(audioEncodeID),m_pVideoSourceQueue(pvideoSourceQueue),m_VideoEncodeID(videoEncodeID),m_rtpPortNum(rtpPortNum),m_rtspPortNum(rtspPortNum),m_ttl(ttl)
{
  m_loopExit = true;
  m_rtpEnableRTSP = true;
  m_initFlag = false;

  if(m_pMediaState)
  {
    m_pMediaState->registerObserver(this);
  }
}

CCreateRTPServer::CCreateRTPServer(CMediaState *ptrState, CPacketQueue *pSourceQueue, enum AVCodecID EncodecID, unsigned short rtpPortNum, unsigned short rtspPortNum , unsigned char ttl):m_pMediaState(ptrState), m_pAudioSourceQueue(NULL),m_AudioEncodeID(AV_CODEC_ID_NONE),m_pVideoSourceQueue(NULL),m_VideoEncodeID(AV_CODEC_ID_NONE),m_rtpPortNum(rtpPortNum),m_rtspPortNum(rtspPortNum),m_ttl(ttl)
{
  switch(EncodecID)
  {
  case AV_CODEC_ID_MP2:
    m_AudioEncodeID = AV_CODEC_ID_MP2;
    break;
  case AV_CODEC_ID_MP3:
    m_AudioEncodeID = AV_CODEC_ID_MP3;
    break;
  case AV_CODEC_ID_AC3:
    m_AudioEncodeID = AV_CODEC_ID_AC3;
    break;
  case AV_CODEC_ID_AAC:
    m_AudioEncodeID = AV_CODEC_ID_AAC;
    break;
  case AV_CODEC_ID_MPEG1VIDEO:
    m_VideoEncodeID = AV_CODEC_ID_MPEG1VIDEO;
    break;
  case AV_CODEC_ID_MPEG2VIDEO:
    m_VideoEncodeID = AV_CODEC_ID_MPEG2VIDEO;
    break;
  case AV_CODEC_ID_MPEG4:
    m_VideoEncodeID = AV_CODEC_ID_MPEG4;
    break;
  case AV_CODEC_ID_H264:
    m_VideoEncodeID = AV_CODEC_ID_H264;
    break;
  default:
    LOG(LOG_TYPE_ERROR, "CCreateRTPServer:not support Media Encode Type!\n");
    break;
  }
  if(m_AudioEncodeID != AV_CODEC_ID_NONE)
    m_pAudioSourceQueue = pSourceQueue;
  else if(m_VideoEncodeID != AV_CODEC_ID_NONE)
    m_pVideoSourceQueue = pSourceQueue;


  m_rtpEnableRTSP = true;
  m_loopExit = true;
  m_initFlag = false;

  if(m_pMediaState)
  {
    m_pMediaState->registerObserver(this);
  }
}
CCreateRTPServer::~CCreateRTPServer()
{
  stopPlay();
}
void CCreateRTPServer::init()
{

  if(!m_initFlag)
  {
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    env = BasicUsageEnvironment::createNew(*scheduler);

    createGroupsock();
    createSourceAndSink();
    createRTCPInstance();
    m_initFlag = true;
  }
}
void CCreateRTPServer::createGroupsock()
{
  sessionState.rtcpGroupsockAudio = NULL;
  sessionState.rtpGroupsockAudio = NULL;
  sessionState.rtcpGroupsockVideo = NULL;
  sessionState.rtpGroupsockVideo = NULL;

  struct in_addr destAddress;
  //destAddress.s_addr = our_inet_addr(m_destAddressStr.c_str());
  if(!m_rtpDestAddressStr.empty())
    destAddress.s_addr = our_inet_addr(m_rtpDestAddressStr.c_str());
  else
    destAddress.s_addr = chooseRandomIPv4SSMAddress(*env);

  if(m_pAudioSourceQueue != NULL)
  {
    const unsigned short rtpPortNumAudio = m_rtpPortNum;
    const unsigned short rtcpPortNumAudio = rtpPortNumAudio + 1;
    const Port rtpPortAudio(rtpPortNumAudio);
    const Port rtcpPortAudio(rtcpPortNumAudio);
    sessionState.rtpGroupsockAudio = new Groupsock(*env, destAddress, rtpPortAudio, m_ttl);
    sessionState.rtcpGroupsockAudio = new Groupsock(*env, destAddress, rtcpPortAudio, m_ttl);
    if(sessionState.rtpGroupsockAudio && sessionState.rtcpGroupsockAudio)
    {
      m_rtpPortNum += 2;
    }
  }

  if(m_pVideoSourceQueue != NULL)
  {
    const unsigned short rtpPortNumVideo = m_rtpPortNum;
    const unsigned short rtcpPortNumVideo = rtpPortNumVideo + 1;
    const Port rtpPortVideo(rtpPortNumVideo);
    const Port rtcpPortVideo(rtcpPortNumVideo);
    sessionState.rtpGroupsockVideo = new Groupsock(*env, destAddress, rtpPortVideo, m_ttl);
    sessionState.rtcpGroupsockVideo = new Groupsock(*env, destAddress, rtcpPortVideo, m_ttl);
  }
}

void CCreateRTPServer::createSourceAndSink()
{
  sessionState.audioSource = NULL;
  sessionState.audioSink = NULL;
  sessionState.videoSource = NULL;
  sessionState.videoSink = NULL;

  if(m_pAudioSourceQueue != NULL)
  {
    while(1)
    {
      if(m_pAudioSourceQueue->Size())
      {
        break;
      }
      else
      {
        usleep(10);
      }
    }

    switch(m_AudioEncodeID)
    {
    case AV_CODEC_ID_MP2:
    case AV_CODEC_ID_MP3:
    {
      CAudioSubSource *paudiosource = CAudioSubSource::createNew(*env, m_pAudioSourceQueue, m_AudioEncodeID);
      sessionState.audioSource = paudiosource;

      sessionState.audioSink = MPEG1or2AudioRTPSink::createNew(*env, sessionState.rtpGroupsockAudio);
    }
      break;
    case AV_CODEC_ID_AAC:
    {
      CAACAudioSource *paacSource = CAACAudioSource::createNew(*env, m_pAudioSourceQueue);
      if(paacSource == NULL)
      {
        LOG(LOG_TYPE_FATAL, "aac Source is NULL(audio Source Queue is Empty!)\n");
        exit(1);
      }

      sessionState.audioSource = paacSource;
      sessionState.audioSink = MPEG4GenericRTPSink::createNew(*env, sessionState.rtpGroupsockAudio, 97, paacSource->samplingFrequency(), "audio", "AAC-hbr", paacSource->configStr(), paacSource->numChannels());
    }
      break;
    case AV_CODEC_ID_AC3:
    {
      CAudioSubSource *paudiosource = CAudioSubSource::createNew(*env, m_pAudioSourceQueue, m_AudioEncodeID);
      if(NULL == paudiosource)
      {
        LOG(LOG_TYPE_FATAL, "create ac3 audio source failed!");
        exit(1);
      }
      sessionState.audioSource = paudiosource;
      sessionState.audioSink = AC3AudioRTPSink::createNew(*env, sessionState.rtpGroupsockAudio, 97, paudiosource->getSamples());
    }
      break;
    default:
      LOG(LOG_TYPE_ERROR,"CCreateRTPServer:unsupported audio encode Type\n");
      break;
    }
  }

  if(m_pVideoSourceQueue != NULL)
  {
    while(1)
    {
      if(m_pVideoSourceQueue->Size())
      {
        break;
      }
      else
      {
        usleep(10);
      }
    }

    OutPacketBuffer::maxSize = 120000;
    FramedSource *pvideoES = NULL;
    switch(m_VideoEncodeID)
    {
    case AV_CODEC_ID_H264:
    {
      pvideoES = CH264VideoSource::createNew(*env, m_pVideoSourceQueue);
      sessionState.videoSource = H264VideoStreamDiscreteFramer::createNew(*env, pvideoES);

      //OutPacketBuffer::maxSize = 100000;
      sessionState.videoSink = H264VideoRTPSink::createNew(*env, sessionState.rtpGroupsockVideo,96);
    }
      break;
    case AV_CODEC_ID_MPEG1VIDEO:
    case AV_CODEC_ID_MPEG2VIDEO:
    {
      pvideoES = CVideoSubSource::createNew(*env, m_pVideoSourceQueue);
      sessionState.videoSource = MPEG1or2VideoStreamDiscreteFramer::createNew(*env, pvideoES);
      sessionState.videoSink = MPEG1or2VideoRTPSink::createNew(*env, sessionState.rtpGroupsockVideo);
    }
      break;
    case AV_CODEC_ID_MPEG4:
    {
      pvideoES = CVideoSubSource::createNew(*env, m_pVideoSourceQueue);
      sessionState.videoSource = MPEG4VideoStreamDiscreteFramer::createNew(*env,pvideoES);
     sessionState.videoSink = MPEG4ESVideoRTPSink::createNew(*env,sessionState.rtpGroupsockVideo, 96);

    }
      break;
    default:
      LOG(LOG_TYPE_ERROR, "CCreateRTPServer: unsupported video encode Type\n");
      break;
    }
  }
}
void CCreateRTPServer::createRTCPInstance()
{
  const unsigned maxCNAMElen = 100;
  unsigned char CNAME[maxCNAMElen+1];
  gethostname((char*)CNAME, maxCNAMElen);
  CNAME[maxCNAMElen] = '\0'; // just in case

  sessionState.audioInstance = NULL;
  sessionState.videoInstance = NULL;

  if(sessionState.audioSink != NULL)
  {
    const unsigned estimatedSessionBandwidthAudio = 160; // in kbps; for RTCP b/w share
    sessionState.audioInstance =  RTCPInstance::createNew(*env, sessionState.rtcpGroupsockAudio, estimatedSessionBandwidthAudio, CNAME, sessionState.audioSink, NULL, False);
  }

  if(sessionState.videoSink != NULL)
  {
    const unsigned estimatedSessionBandwidthVideo = 1000;
    sessionState.videoInstance =  RTCPInstance::createNew(*env, sessionState.rtcpGroupsockVideo, estimatedSessionBandwidthVideo, CNAME, sessionState.videoSink, NULL, False);
  }

  if(m_rtpEnableRTSP)
  {
    rtspServer = new CMyRTSPServer(*env, m_rtspPortNum);

    sms = ServerMediaSession::createNew(*env, "live", "liveMedia", "Video and Audio RTSP Session Stream", False);
    if(NULL == sms)
    {
      cout << "Create serverMediaSession Failed\n" << endl;
      exit(1);
    }

    if(sessionState.videoInstance != NULL)
    {
      sms->addSubsession(PassiveServerMediaSubsession::createNew(*sessionState.videoSink, sessionState.videoInstance));
    }
    if(sessionState.audioInstance != NULL)
    {
      sms->addSubsession(PassiveServerMediaSubsession::createNew(*sessionState.audioSink, sessionState.audioInstance));
    }

    rtspServer->addServerMediaSession(sms);
    char* url = rtspServer->rtspURL(sms);
    *env << "Play this stream using the URL \n\t\"" << url << "\"\n";
    delete[] url;
  }
}
void CCreateRTPServer::doPlay()
{
  init();

  play();

  m_loopExit = false;
  watchLoop = 0;
  env->taskScheduler().doEventLoop(&watchLoop);
  m_loopExit = true;
}

void CCreateRTPServer::play()
{
  if(sessionState.videoSource != NULL)
  {
    *env << "Playing Video.....\n";
    sessionState.videoSink->startPlaying(*sessionState.videoSource, NULL, NULL);
  }
  if(sessionState.audioSource != NULL)
  {
    *env << "Playing Audio.....\n";
    sessionState.audioSink->startPlaying(*sessionState.audioSource, NULL, NULL);
  }
}

void CCreateRTPServer::stopPlay()
{
  if(sessionState.audioSink != NULL)
  {
    sessionState.audioSink->stopPlaying();
  }
  if(sessionState.videoSink != NULL)
  {
    sessionState.videoSink->stopPlaying();
  }
  stopLoop();
  uninit();
}

void CCreateRTPServer::update(MEDIA_STATE_ENUM state)
{
  this->m_state = state;
  if(this->m_state == STATE_STOP)
  {
    cout << "update STATE_STOP" << endl;
    stopPlay();
  }
}

void CCreateRTPServer::uninit()
{
  while(!m_loopExit)
  {
    usleep(1);
  }
  if(m_rtpEnableRTSP)
  {
    rtspServer->deleteServerMediaSession(sms);
    sms->deleteAllSubsessions();
    delete rtspServer;
    rtspServer = NULL;
  }
  if(sessionState.audioSource != NULL)
  {
    Medium::close(sessionState.audioSource);
  }
  if(sessionState.videoSource != NULL)
  {
    Medium::close(sessionState.videoSource);
  }
  if(sessionState.audioInstance != NULL)
  {
    Medium::close(sessionState.audioInstance);
  }
  if(sessionState.videoInstance != NULL)
  {
    Medium::close(sessionState.videoInstance);
  }
  if(sessionState.rtpGroupsockAudio != NULL)
  {
    delete sessionState.rtpGroupsockAudio;
    sessionState.rtpGroupsockAudio = NULL;
  }
  if(sessionState.rtcpGroupsockAudio != NULL)
  {
    delete sessionState.rtcpGroupsockAudio;
    sessionState.rtcpGroupsockAudio = NULL;
  }
  if(sessionState.rtpGroupsockVideo != NULL)
  {
    delete sessionState.rtpGroupsockVideo;
    sessionState.rtpGroupsockVideo = NULL;
  }
  if(sessionState.rtcpGroupsockVideo != NULL)
  {
    delete sessionState.rtcpGroupsockVideo;
    sessionState.rtcpGroupsockVideo = NULL;
  }
  env->reclaim();
}
void CCreateRTPServer::stopLoop()
{
  watchLoop = ~0;

  if(m_state != STATE_STOP)
  {
    m_pMediaState->setState(STATE_STOP);
  }
}

void CCreateRTPServer::setRTPAddress(const string &addr)
{
  m_rtpDestAddressStr = addr;
}
