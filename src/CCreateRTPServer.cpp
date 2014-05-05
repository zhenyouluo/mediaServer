#include "CCreateRTPServer.h"
#include "CH264VideoSubSource.h"
#include "CMP2orMP3orAC3AudioSubSource.h"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
#include "CMPEGVideoSubSource.h"
#include "CAACAudioSubSource.h"


CCreateRTPServer::CCreateRTPServer(CMediaState *ptrState, CPacketQueue *paudioSourceQueue,
                                   enum AVCodecID audioEncodeID, CPacketQueue *pvideoSourceQueue,
                                   enum AVCodecID videoEncodeID,unsigned short rtpPortNum,
                                   unsigned short rtspPortNum, unsigned char ttl)
    :m_pMediaState(ptrState), m_pAudioSourceQueue(paudioSourceQueue),
      m_AudioEncodeID(audioEncodeID),m_pVideoSourceQueue(pvideoSourceQueue),
      m_VideoEncodeID(videoEncodeID),m_rtpPortNum(rtpPortNum),
      m_rtspPortNum(rtspPortNum),m_ttl(ttl)
{
    m_loopExit = true;
    m_rtpEnableRTSP = true;
    m_initFlag = false;

    if(m_pMediaState)
    {
        m_pMediaState->registerObserver(this);
    }
}

CCreateRTPServer::CCreateRTPServer(CMediaState *ptrState, CPacketQueue *pSourceQueue,
                                   enum AVCodecID EncodecID, unsigned short rtpPortNum,
                                   unsigned short rtspPortNum , unsigned char ttl)
    :m_pMediaState(ptrState), m_pAudioSourceQueue(NULL),m_AudioEncodeID(AV_CODEC_ID_NONE),
      m_pVideoSourceQueue(NULL),m_VideoEncodeID(AV_CODEC_ID_NONE),m_rtpPortNum(rtpPortNum),
      m_rtspPortNum(rtspPortNum),m_ttl(ttl)
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
    m_prtcpGroupsockAudio = NULL;
    m_prtpGroupsockAudio = NULL;
    m_prtcpGroupsockVideo = NULL;
    m_prtpGroupsockVideo = NULL;

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
        m_prtpGroupsockAudio = new Groupsock(*env, destAddress, rtpPortAudio, m_ttl);
        m_prtcpGroupsockAudio = new Groupsock(*env, destAddress, rtcpPortAudio, m_ttl);
        if(m_prtpGroupsockAudio && m_prtcpGroupsockAudio)
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
        m_prtpGroupsockVideo = new Groupsock(*env, destAddress, rtpPortVideo, m_ttl);
        m_prtcpGroupsockVideo = new Groupsock(*env, destAddress, rtcpPortVideo, m_ttl);
    }
}

void CCreateRTPServer::createSourceAndSink()
{
    m_paudioSource = NULL;
    m_paudioSink = NULL;
    m_pvideoSource = NULL;
    m_pvideoSink = NULL;

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
            CMP2orMP3orAC3AudioSubSource *paudiosource = CMP2orMP3orAC3AudioSubSource::
                    createNew(*env, m_pAudioSourceQueue, m_AudioEncodeID);
            m_paudioSource = paudiosource;

            m_paudioSink = MPEG1or2AudioRTPSink::createNew(*env, m_prtpGroupsockAudio);
        }
            break;
        case AV_CODEC_ID_AAC:
        {
            CAACAudioSubSource *paacSource = CAACAudioSubSource::createNew(*env, m_pAudioSourceQueue);
            if(paacSource == NULL)
            {
                LOG(LOG_TYPE_FATAL, "aac Source is NULL(audio Source Queue is Empty!)\n");
                exit(1);
            }

            m_paudioSource = paacSource;
            m_paudioSink = MPEG4GenericRTPSink::createNew(*env, m_prtpGroupsockAudio,
                                                          97, paacSource->samplingFrequency(),
                                                          "audio", "AAC-hbr", paacSource->configStr(),
                                                          paacSource->numChannels());
        }
            break;
        case AV_CODEC_ID_AC3:
        {
            CMP2orMP3orAC3AudioSubSource *paudiosource = CMP2orMP3orAC3AudioSubSource::
                    createNew(*env, m_pAudioSourceQueue, m_AudioEncodeID);
            if(NULL == paudiosource)
            {
                LOG(LOG_TYPE_FATAL, "create ac3 audio source failed!");
                exit(1);
            }
            m_paudioSource = paudiosource;
            m_paudioSink = AC3AudioRTPSink::createNew(*env, m_prtpGroupsockAudio,
                                                      97, paudiosource->getSamples());
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
            pvideoES = CH264VideoSubSource::createNew(*env, m_pVideoSourceQueue);
            m_pvideoSource = H264VideoStreamDiscreteFramer::createNew(*env, pvideoES);

            m_pvideoSink = H264VideoRTPSink::createNew(*env, m_prtpGroupsockVideo,96);
        }
            break;
        case AV_CODEC_ID_MPEG1VIDEO:
        case AV_CODEC_ID_MPEG2VIDEO:
        {
            pvideoES = CMPEGVideoSubSource::createNew(*env, m_pVideoSourceQueue);
            m_pvideoSource = MPEG1or2VideoStreamDiscreteFramer::createNew(*env, pvideoES);
            m_pvideoSink = MPEG1or2VideoRTPSink::createNew(*env, m_prtpGroupsockVideo);
        }
            break;
        case AV_CODEC_ID_MPEG4:
        {
            pvideoES = CMPEGVideoSubSource::createNew(*env, m_pVideoSourceQueue);
            m_pvideoSource = MPEG4VideoStreamDiscreteFramer::createNew(*env,pvideoES);
            m_pvideoSink = MPEG4ESVideoRTPSink::createNew(*env, m_prtpGroupsockVideo, 96);

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

    m_paudioInstance = NULL;
    m_pvideoInstance = NULL;

    if(m_paudioSink != NULL)
    {
        const unsigned estimatedSessionBandwidthAudio = 160; // in kbps; for RTCP b/w share
        m_paudioInstance =  RTCPInstance::createNew(*env, m_prtcpGroupsockAudio,
                                                    estimatedSessionBandwidthAudio, CNAME,
                                                    m_paudioSink, NULL, False);
    }

    if(m_pvideoSink != NULL)
    {
        const unsigned estimatedSessionBandwidthVideo = 1000;
        m_pvideoInstance =  RTCPInstance::createNew(*env, m_prtcpGroupsockVideo,
                                                    estimatedSessionBandwidthVideo, CNAME,
                                                    m_pvideoSink, NULL, False);
    }

    if(m_rtpEnableRTSP)
    {
        m_pRtspServer = new CMyRTSPServer(*env, m_rtspPortNum);

        m_sms = ServerMediaSession::createNew(*env, "live", "liveMedia", "Video and Audio RTSP Session Stream",
                                              False);
        if(NULL == m_sms)
        {
            cout << "Create serverMediaSession Failed\n" << endl;
            exit(1);
        }

        if(m_pvideoInstance != NULL)
        {
            m_sms->addSubsession(PassiveServerMediaSubsession::createNew(*m_pvideoSink,
                                                                         m_pvideoInstance));
        }
        if(m_paudioInstance != NULL)
        {
            m_sms->addSubsession(PassiveServerMediaSubsession::createNew(*m_paudioSink,
                                                                         m_paudioInstance));
        }

        m_pRtspServer->addServerMediaSession(m_sms);
        char* url = m_pRtspServer->rtspURL(m_sms);
        m_rtspPlayAddress = url;
        *env << "Play this stream using the URL \n\t\"" << m_rtspPlayAddress.c_str() << "\"\n";
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
    if(m_pvideoSource != NULL)
    {
        *env << "Playing Video.....\n";
        m_pvideoSink->startPlaying(*m_pvideoSource, NULL, NULL);
    }
    if(m_paudioSource != NULL)
    {
        *env << "Playing Audio.....\n";
        m_paudioSink->startPlaying(*m_paudioSource, NULL, NULL);
    }
}

void CCreateRTPServer::stopPlay()
{
    if(m_paudioSink != NULL)
    {
        m_paudioSink->stopPlaying();
    }
    if(m_pvideoSink != NULL)
    {
        m_pvideoSink->stopPlaying();
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
        if(m_sms)
        {
            m_pRtspServer->deleteServerMediaSession(m_sms);
            m_sms->deleteAllSubsessions();
        }

        delete m_pRtspServer;
        m_pRtspServer = NULL;
    }
    if(m_paudioSource != NULL)
    {
        Medium::close(m_paudioSource);
    }
    if(m_pvideoSource != NULL)
    {
        Medium::close(m_pvideoSource);
    }
    if(m_paudioInstance != NULL)
    {
        Medium::close(m_paudioInstance);
    }
    if(m_pvideoInstance != NULL)
    {
        Medium::close(m_pvideoInstance);
    }
    if(m_prtpGroupsockAudio != NULL)
    {
        delete m_prtpGroupsockAudio;
        m_prtpGroupsockAudio = NULL;
    }
    if(m_prtcpGroupsockAudio != NULL)
    {
        delete m_prtcpGroupsockAudio;
        m_prtcpGroupsockAudio = NULL;
    }
    if(m_prtpGroupsockVideo != NULL)
    {
        delete m_prtpGroupsockVideo;
        m_prtpGroupsockVideo = NULL;
    }
    if(m_prtcpGroupsockVideo != NULL)
    {
        delete m_prtcpGroupsockVideo;
        m_prtcpGroupsockVideo = NULL;
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
string CCreateRTPServer::rtspPlayAddress()const
{
    return m_rtspPlayAddress;
}
