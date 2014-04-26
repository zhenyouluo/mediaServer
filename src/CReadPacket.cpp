#include "CReadPacket.h"
#include<unistd.h>
CReadPacket::CReadPacket(CMediaState *ptrSub, const char *pfileName)
{
  if(!ptrSub)
  {
    exit(1);
  }

  m_pSubState = ptrSub;
  m_pSubState->registerObserver(this);


  m_pDemux = new CMediaDemux(pfileName);
  if(!m_pDemux)
  {
    exit(1);
  }

  m_pSrcFileName = pfileName;
  m_state = STATE_STOP;
}
CReadPacket::~CReadPacket()
{
  if (m_pDemux)
  {
    delete m_pDemux;
    m_pDemux = NULL;
  }
}

void CReadPacket::update(MEDIA_STATE_ENUM state)
{
  m_state = state;
}

void CReadPacket::readPacket()
{

  int ret = 0;
  struct AVPacket pkt;
  av_init_packet(&pkt);
  pkt.data = NULL;
  pkt.size = 0;

  while (m_state == STATE_RUNNING)
  {

    if(m_liveAudioMediaSource.Size() > 100 || m_liveVideoMediaSource.Size() > 100)
    {
      usleep(50);
      continue;
    }

    ret = m_pDemux->readPacket(pkt);

    if (ret >= 0)
    {
      if (pkt.stream_index == m_pDemux->videoStreamIndex())
      {
        m_liveVideoMediaSource.Push(&pkt);
      }
      else if (pkt.stream_index == m_pDemux->audioStreamIndex())
      {
        m_liveAudioMediaSource.Push(&pkt);
      }
      //cout << "CReadPacket  readPacket " << endl;
    }
    else
    {
      cout << "CReadPacket  readPacket return error or End of file!" << endl;
      if (m_pDemux)
      {
        delete m_pDemux;
        m_pDemux = NULL;
      }
      m_pDemux = new CMediaDemux(m_pSrcFileName);
      if(NULL == m_pDemux)
      {
          cout << "reopen file failed!" << endl;
          exit(1);
      }
     // m_pSubState->setState(STATE_INPUT_END);
    }
  }
}

CPacketQueue* CReadPacket::videoMediaSourceQueue()
{
  if(-1 == m_pDemux->videoStreamIndex())
  {
    return NULL;
  }
  return &m_liveVideoMediaSource;
}
CPacketQueue* CReadPacket::audioMediaSourceQueue()
{
  if(-1 == m_pDemux->audioStreamIndex())
  {
    return NULL;
  }
  return &m_liveAudioMediaSource;
}

enum AVCodecID CReadPacket::audioCodeId()const
{
  if(m_pDemux)
  {
    return m_pDemux->audioCodeId();
  }
  return AV_CODEC_ID_NONE;
}
enum AVCodecID CReadPacket::videoCodeId()const
{
  if(m_pDemux)
  {
    return m_pDemux->videoCodeId();
  }
  return AV_CODEC_ID_NONE;
}
double CReadPacket::videoFPS()const
{
  if(m_pDemux)
  {
    return m_pDemux->videoFPS();
  }
  return 0;
}
