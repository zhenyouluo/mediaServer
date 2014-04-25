#ifndef CVIDEOESUBSESSION_H
#define CVEVIDEOESUBSESSION_H

#include "liveMedia.hh"
#include "CPacketQueue.h"
#include "CVideoSubSource.h"
#include "CH264VideoSource.h"

class CVideoSubsession:public OnDemandServerMediaSubsession
{
public:
  static CVideoSubsession* createNew(UsageEnvironment &env, CPacketQueue *videoSourceQueue, enum AVCodecID videoEncodeType);

protected:
  CVideoSubsession(UsageEnvironment &env, CPacketQueue *videoSourceQueue, enum AVCodecID videoEncodeType);
	virtual ~CVideoSubsession();

protected: // redefined virtual functions
	virtual FramedSource* createNewStreamSource(unsigned clientSessionId,unsigned& estBitrate) ;
	// "estBitrate" is the stream's estimated bitrate, in kbps
	virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);
private:
	FramedSource* createStreamSource(FramedSource *framedSubSource, unsigned &estBitrate);
	RTPSink* createRTPSink(Groupsock* rtpGroupsock,unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);
private:
	CPacketQueue *m_pvideoSourceQueue;
  enum AVCodecID m_videoEncodeType;
};

#endif
