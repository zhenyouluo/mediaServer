#include "CMediaDemux.h"
#include <iostream>
using namespace std;

CMediaDemux::CMediaDemux(const char *fileName)
  :m_pFormatCtx(NULL),m_pvideoCodec(NULL),m_pvideoCodecContext(NULL),
    m_paudioCodec(NULL), m_paudioCodecContext(NULL),m_audioCodeId(AV_CODEC_ID_NONE),
    m_videoCodeId(AV_CODEC_ID_NONE),m_pSrcFile(fileName),m_notSetUp(true)
{
  //nothing to do;
  setUp();
}

CMediaDemux::~CMediaDemux()
{
  unSetUp();
}

void CMediaDemux::setUp()
{
  if(!m_notSetUp)
    return;

  av_register_all();
  avformat_network_init();

  //av_init_packet(&AvPacket);//Initialize optional fields of a packet with default values.
  cout << "CMediaDemux setUp" << endl;

  if (avformat_open_input(&m_pFormatCtx, m_pSrcFile, NULL, NULL) != 0)
  {
    cerr << "Open file Failure!\n";
    exit(1);

  }
  // Retrieve stream information

  if(avformat_find_stream_info(m_pFormatCtx, NULL)<0)
  {
    cerr << "Couldn't find stream information!\n";
    exit(1);
  }
  av_dump_format(m_pFormatCtx, 0, m_pSrcFile, 0);

  m_videoStreamIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1,-1,&m_pvideoCodec,0);
  if (!((m_videoStreamIndex >= 0)&&(m_pvideoCodec != NULL)))
  {
    m_videoStreamIndex = -1;
    m_videoCodeId = AV_CODEC_ID_NONE;
    cerr << "Could not find " << av_get_media_type_string(AVMEDIA_TYPE_VIDEO) << " stream in file "<< m_pSrcFile << endl;
  }
  else
  {
    m_pvideoCodecContext = m_pFormatCtx->streams[m_videoStreamIndex]->codec;
    m_pvideoCodecContext->codec = m_pvideoCodec;
    m_videoCodeId = m_pvideoCodec->id;

    m_videoFPS = av_q2d(m_pFormatCtx->streams[m_videoStreamIndex]->avg_frame_rate);

    if (avcodec_open2(m_pvideoCodecContext, m_pvideoCodec, NULL) < 0)
    {
      cerr << "Failed to open " << av_get_media_type_string(AVMEDIA_TYPE_VIDEO) << " codec" << endl;
      exit(1);
    }
  }
  
  m_audioStreamIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_AUDIO, -1,-1,&m_paudioCodec,0);
  if (!((m_audioStreamIndex >= 0)&&(m_paudioCodec != NULL)))
  {
    m_audioStreamIndex = -1;
    m_audioCodeId = AV_CODEC_ID_NONE;
    cerr << "Could not find " << av_get_media_type_string(AVMEDIA_TYPE_AUDIO) << " stream in file "<< m_pSrcFile << endl;
  }
  else
  {
    m_paudioCodecContext = m_pFormatCtx->streams[m_audioStreamIndex]->codec;
    m_paudioCodecContext->codec = m_paudioCodec;
    m_audioCodeId = m_paudioCodec->id;
    if (avcodec_open2(m_paudioCodecContext, m_paudioCodec, NULL) < 0)
    {
      cerr << "Failed to open " << av_get_media_type_string(AVMEDIA_TYPE_VIDEO) << " codec" << endl;
      exit(1);
    }
  }


  m_notSetUp = false;
  return ;
}

void CMediaDemux::unSetUp()
{
    /*
  avcodec_close(m_pvideoCodecContext);
  cout << "1///////////////////////////";
  av_free(m_pvideoCodecContext);
  cout << "2///////////////////////////";
  avcodec_close(m_paudioCodecContext);
  cout << "3///////////////////////////";
  av_free(m_paudioCodecContext);
cout << "4///////////////////////////";
*/
  avformat_close_input(&m_pFormatCtx);
  cout << "5///////////////////////////";
}

int CMediaDemux::readPacket(AVPacket &pkt)
{
  if(m_notSetUp)
  {
    setUp();
  }
  int ret;
  ret = av_read_frame(m_pFormatCtx, &pkt);
  return ret;
}
