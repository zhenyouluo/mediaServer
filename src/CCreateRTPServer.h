#ifndef CCREATERTPSERVER_H
#define CCREATERTPSERVER_H

#include "liveMedia.hh"
#include "CPacketQueue.h"
#include "CLog.h"

#include <sstream>
#include <iostream>
#include <string>

#include"CMyRTSPServer.h"
#include"CMediaState.h"

#include"HelpFunc.h"

#define IMPLEMENT_RTSPSERVER 1


struct sessionState_t {
  FramedSource *audioSource;
  FramedSource *videoSource;
  Groupsock *rtpGroupsockAudio;
  Groupsock *rtcpGroupsockAudio;
  Groupsock *rtpGroupsockVideo;
  Groupsock *rtcpGroupsockVideo;
  RTPSink *audioSink;
  RTPSink *videoSink;
  RTCPInstance *videoInstance;
  RTCPInstance *audioInstance;
};

class CCreateRTPServer:public CObserver
{
public:
  /*
  * 支持音频编码格式：
  *   CODEC_ID_AC3,
  *   CODEC_ID_AAC
  *   CODEC_ID_MP2,
  *   CODEC_ID_MP3,
  * 支持视频编码格式：
  *   CODEC_ID_MPEG1VIDEO,
  *   CODEC_ID_MPEG2VIDEO,
  *   CODEC_ID_MPEG4,
  *   CODEC_ID_H264
  */
  //this Constructor for RTSP
  CCreateRTPServer(CMediaState *ptrState, CPacketQueue *paudioSourceQueue, enum AVCodecID audioEncodeID, CPacketQueue *pvideoSourceQueue, enum AVCodecID videoEncodeID, unsigned short rtpPortNum, unsigned short rtspPortNum, unsigned char ttl = 255);

  //this Constructor for  Audio or Video only.
  CCreateRTPServer(CMediaState *ptrState,CPacketQueue *pSourceQueue, enum AVCodecID EncodeID, unsigned short rtpPortNum, unsigned short rtspPortNum , unsigned char ttl = 255);

  ~CCreateRTPServer();

  void doPlay();
  void stopPlay();
  void play();
  void update(MEDIA_STATE_ENUM state);
  void setRTPAddress(const string &addr);
private:
  void init();
  void uninit();
  void stopLoop();
  void createGroupsock();
  void createSourceAndSink();
  void createRTCPInstance();

private:
  char watchLoop;
  bool m_loopExit;
  MEDIA_STATE_ENUM m_state;
  CMediaState *m_pMediaState;
  CPacketQueue *m_pAudioSourceQueue;
  enum AVCodecID m_AudioEncodeID;
  CPacketQueue *m_pVideoSourceQueue;
  enum AVCodecID m_VideoEncodeID;

  UsageEnvironment *env;
  unsigned short m_rtpPortNum;
  unsigned short m_rtspPortNum;
  unsigned char m_ttl;
  string m_rtpDestAddressStr;

  bool m_rtpEnableRTSP;
  bool m_initFlag;
  sessionState_t sessionState;
  CMyRTSPServer *rtspServer;
  ServerMediaSession *sms;
};
#endif

