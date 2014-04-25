#ifndef CAUDIOSUBSESSION_H
#define CAUDIOSUBSESSION_H

#include "liveMedia.hh"
#include "CPacketQueue.h"
#include "CAudioSubSource.h"
#include "CAACAudioSource.h"

class CAudioSubsession:public OnDemandServerMediaSubsession
{
public:
  static CAudioSubsession* createNew(UsageEnvironment &env, CPacketQueue *audioSourceQueue, enum AVCodecID audioType);
protected:
  CAudioSubsession(UsageEnvironment &env, CPacketQueue *audioSourceQueue, enum AVCodecID audioType);
	~CAudioSubsession();

protected: // redefined virtual functions
	virtual FramedSource* createNewStreamSource(unsigned clientSessionId,unsigned& estBitrate) ;
	// "estBitrate" is the stream's estimated bitrate, in kbps, 
	virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);

private:
	FramedSource* createStreamSource(FramedSource* FramedSbusource, unsigned& estBitrate);
	RTPSink* createRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);

private:
	CPacketQueue *faudioSourceQueue;
  enum AVCodecID faudioType;
};

#endif
