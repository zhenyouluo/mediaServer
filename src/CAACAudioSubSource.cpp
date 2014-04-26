#include "CAACAudioSubSource.h"

static unsigned const samplingFrequencyTable[16] = {
  96000, 88200, 64000, 48000,
  44100, 32000, 24000, 22050,
  16000, 12000, 11025, 8000,
  7350, 0, 0, 0
};

CAACAudioSubSource *CAACAudioSubSource::createNew(UsageEnvironment &env, CPacketQueue *dataPacketQueue)
{
  CAACAudioSubSource *ptrSource = NULL;

  if(dataPacketQueue->Size() != 0)
  {
    AVPacket* pPkt = NULL;
    pPkt = dataPacketQueue->Front();
    if(pPkt != NULL)
    {
      unsigned char headers[4];
      memcpy(headers, pPkt->data, sizeof(headers));
      u_int8_t sampling_frequency_index = (headers[2]&0x3c)>>2;
      if(sampling_frequency_index > 12)
      {
        env.setResultMsg("AAC Audio Source's Sampling frequency is 0!");
        return ptrSource;
      }
      u_int8_t channelConfiguration = ((headers[2]&0x01)<<2)|((headers[3]&0xC0)>>6); // 3 bits
      // Construct the 'AudioSpecificConfig', and from it, the corresponding ASCII string:
      u_int8_t profile = (headers[2]&0xC0)>>6; // 2 bits
      if (profile == 3)
      {
        env.setResultMsg("Bad (reserved) 'profile': 3 in first frame of ADTS file");
        return ptrSource;
      }
      ptrSource =  new CAACAudioSubSource(env, dataPacketQueue, sampling_frequency_index, channelConfiguration, profile);
    }

  }
  return ptrSource;
}
CAACAudioSubSource::CAACAudioSubSource(UsageEnvironment &env, CPacketQueue *dataPacketQueue, u_int8_t sampling_frequency_index,
                                 u_int8_t channelConfiguration, u_int8_t profile)
                                :CCMediaSource(env, dataPacketQueue)
{
  m_samplingFrequency = samplingFrequencyTable[sampling_frequency_index];

  cout << "AACAudioSource fSamplingFrequency is " << m_samplingFrequency << endl;
  m_uSecsPerFrame = ((1024/*samples-per-frame*/*1000000 *2) / m_samplingFrequency/*samples-per-second*/+1)/2;//rounds to nearest integer

  m_numChannels = channelConfiguration == 0 ? 2 : channelConfiguration;
  unsigned char audioSpecificConfig[2];
  u_int8_t const audioObjectType = profile + 1;
  audioSpecificConfig[0] = (audioObjectType<<3) | (sampling_frequency_index>>1);
  audioSpecificConfig[1] = (sampling_frequency_index<<7) | (channelConfiguration<<3);
  sprintf(m_configStr, "%02X%02x", audioSpecificConfig[0], audioSpecificConfig[1]);
}
CAACAudioSubSource::~CAACAudioSubSource()
{
  //nothing
}

bool CAACAudioSubSource::deliverFrame()
{
  AVPacket avk;
  unsigned char headers[7];

  if(m_pMediaSourceQueue->Pop(&avk))
  {
    int header_size = sizeof(headers);
    memcpy(headers, avk.data, header_size);

    fFrameSize = avk.size > header_size ? avk.size - header_size : 0;

    // Extract important fields from the headers:
    Boolean protection_absent = headers[1]&0x01;
    unsigned skip = 0;
    // skip "src_check" filed
    if (!protection_absent)
    {
      skip = 2;
      fFrameSize = fFrameSize > skip ? fFrameSize - skip : 0;
    }
    if(fFrameSize > fMaxSize)
    {
      fNumTruncatedBytes = fFrameSize - fMaxSize;
      LOG(LOG_TYPE_NOTICE, "CAACAudioSource Frame fNumTruncatedBytes: %d Bytes!\n",fNumTruncatedBytes);
      fFrameSize = fMaxSize;
    }
    else
    {
      fNumTruncatedBytes = 0;
    }

    memmove(fTo, avk.data + sizeof(headers) + skip , fFrameSize);
    // Set the 'presentation time':
    if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0)
    {
      // This is the first frame, so use the current time:
      gettimeofday(&fPresentationTime, NULL);
    }
    else
    {
      // Increment by the play time of the previous frame:
      unsigned uSeconds = fPresentationTime.tv_usec + m_uSecsPerFrame;
      fPresentationTime.tv_sec += uSeconds/1000000;
      fPresentationTime.tv_usec = uSeconds%1000000;
    }
    fDurationInMicroseconds = m_uSecsPerFrame;
    av_free_packet(&avk);
    nextTask() = envir().taskScheduler().scheduleDelayedTask(0,(TaskFunc*)FramedSource::afterGetting, this);
    return true;
  }
  return false;
}


