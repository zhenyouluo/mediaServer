#ifndef CREADPACKET_H
#define CREADPACKET_H

#include "CMediaDemux.h"
#include "ffmpeg.h"
#include "CPacketQueue.h"
#include "CMediaState.h"
class CReadPacket:public CObserver
{
public:
  CReadPacket(CMediaState *ptrSub, const char *pFileName);
	~CReadPacket();
  void update(MEDIA_STATE_ENUM state);

	void readPacket();

  CPacketQueue* videoMediaSourceQueue();
  CPacketQueue* audioMediaSourceQueue();

  enum AVCodecID audioCodeId()const;
  enum AVCodecID videoCodeId()const;
  double videoFPS()const;

private:
  CMediaDemux *m_pDemux;

  CPacketQueue m_liveVideoMediaSource;
  CPacketQueue m_liveAudioMediaSource;

  MEDIA_STATE_ENUM m_state;
  CMediaState *m_pSubState;
  const char *m_pSrcFileName;
};
#endif
