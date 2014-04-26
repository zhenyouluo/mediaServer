#include "CMP2orMP3orAC3AudioSubSource.h"

//work for mp2 and mp3 audio frame!
static unsigned getMP2orMP3SamplingFreq(AVPacket *MP2orMP3Packet)
{
  unsigned char syncinfo[4];
  memcpy(syncinfo, MP2orMP3Packet->data, sizeof(syncinfo));
  unsigned char byte;
  unsigned samplingFreq = 0;
  byte = syncinfo[2];
  unsigned char samplingFreqIndex = (byte&0x0C) >> 2;
  switch(samplingFreqIndex)
  {
  case 0:
    samplingFreq = 44100;
    break;
  case 1:
    samplingFreq = 48000;
    break;
  case 2:
    samplingFreq = 32000;
    break;
  }
  LOG(LOG_TYPE_NOTICE, "getMP2orMP3SamplingFreq: samplingFreq = %d\n" ,samplingFreq);
  cout << "getMP2orMP3SamplingFreq: samplingFreq == " << samplingFreq << endl;
  return samplingFreq;
}

//only work for ac3 audio frame!
static unsigned getAC3SamplingFreq(AVPacket *ac3packet)
{
  unsigned char syncinfo[5];
  memcpy(syncinfo, ac3packet->data, sizeof(syncinfo));

  //ac3 audio header (first 2 bytes == 0x0B77) at the start.
  if(!((syncinfo[0] == 0x0B)&&(syncinfo[1] == 0x77)))
  {
    LOG(LOG_TYPE_ERROR,"getAC3SamplingFreq:ac3 audio frame header first 2 bytes != 0x0B77");
    return 0;

  }
  unsigned char bytes = syncinfo[4];
  unsigned samplingFreq = 0;

  unsigned char samplingFreqIndex = (bytes&0xC0) >> 6;
  switch(samplingFreqIndex)
  {
  case 0:
    samplingFreq = 48000;
    break;
  case 1:
    samplingFreq = 44100;
    break;
  case 2:
  case 3://reserved
    samplingFreq = 32000;
    break;
  }
  LOG(LOG_TYPE_NOTICE, "getAC3SamplingFreq: samplingFreq = %d\n" ,samplingFreq);
  cout << "getAC3SamplingFreq: samplingFreq == " << samplingFreq << endl;
  return samplingFreq;
}

CMP2orMP3orAC3AudioSubSource* CMP2orMP3orAC3AudioSubSource::createNew(UsageEnvironment &env, CPacketQueue *mediaSourceQueue, enum AVCodecID encodeID)
{
  unsigned uSecsPerFrame = 0;
  unsigned numSamplesPerFrame = 0;
  //uSecsPerFrame = (samples-per-frame*1000000)/samples-per-second ;
  //mp2 and mp3 samples-per-frame is 1152, ac3 is 1536
  if((encodeID == AV_CODEC_ID_MP2)||(encodeID == AV_CODEC_ID_MP3))
  {
    numSamplesPerFrame = getMP2orMP3SamplingFreq(mediaSourceQueue->Front());

    switch(numSamplesPerFrame)
    {
    case 48000:
      uSecsPerFrame = 24000;
      break;
    case 44100:
      uSecsPerFrame = 26122;
      break;
    case 32000:
      uSecsPerFrame = 36000;
      break;
    }
  }
  else if(encodeID == AV_CODEC_ID_AC3)
  {
    numSamplesPerFrame = getAC3SamplingFreq(mediaSourceQueue->Front());
    switch(numSamplesPerFrame)
    {
    case 48000:
      uSecsPerFrame = 32000;
      break;
    case 44100:
      uSecsPerFrame = 34830;
      break;
    case 32000:
      uSecsPerFrame = 48000;
      break;
    }
  }
  else
  {
    LOG(LOG_TYPE_ERROR,"CAudioSubSource::createNew: the audio encodeID is  unsupported!\n");
    cout << "CAudioSubSource::createNew: the audio encodeID is  unsupported!\n";
    return NULL;
  }
  cout <<"numSamplesPerFrame == "<< numSamplesPerFrame << "\t"<< "uSecsPerFrame == " << uSecsPerFrame << endl;
  return new CMP2orMP3orAC3AudioSubSource(env, mediaSourceQueue,numSamplesPerFrame, uSecsPerFrame);
}

CMP2orMP3orAC3AudioSubSource::CMP2orMP3orAC3AudioSubSource(UsageEnvironment &env,
                                                           CPacketQueue *mediaSourceQueue,
                                                           unsigned sampleFreq, unsigned uSecsPerFrame)
    :CCMediaSource(env, mediaSourceQueue), m_samplesFreq(sampleFreq),m_uSecsPerFrame(uSecsPerFrame)
{
}
CMP2orMP3orAC3AudioSubSource::~CMP2orMP3orAC3AudioSubSource()
{
}

bool CMP2orMP3orAC3AudioSubSource::deliverFrame()
{
  AVPacket tempPkt;

  if(m_pMediaSourceQueue->Pop(&tempPkt))
  {
    fFrameSize = tempPkt.size;
    if (fFrameSize > fMaxSize)
    {
      fNumTruncatedBytes = fFrameSize - fMaxSize;
      LOG(LOG_TYPE_NOTICE, "CAudioSubSource Frame fNumTruncatedBytes:%d Bytes!\n",fNumTruncatedBytes);
      fFrameSize = fMaxSize;
    }
    else
    {
      fNumTruncatedBytes = 0;
    }
    memmove(fTo, tempPkt.data, fFrameSize);

    av_free_packet(&tempPkt);
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

    nextTask() = envir().taskScheduler().scheduleDelayedTask(0,(TaskFunc*)FramedSource::afterGetting, this);
    return true;
  }

  return false;
}


