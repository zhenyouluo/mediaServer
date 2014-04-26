#include "CServer.h"

void* ThreadFunc(void *arg)
{
  CReadPacket *liveSource = (CReadPacket*)arg;

  liveSource->readPacket();
  return 0;
}

CServer::CServer(const string &inputFile, unsigned short rtpNum, unsigned short rtspNum,
                 unsigned char ttl)
  :m_pServer(NULL), m_pInput(NULL)
{
  m_pMediaState = new CMediaState;
  if(NULL == m_pMediaState)
  {
    cout << "Create MediaState Object failed" << endl;
    exit(1);
  }

  const char *ptr = inputFile.c_str();
  m_pInput = new CReadPacket(m_pMediaState, ptr);
  if(NULL == m_pInput)
  {
    cout << "Create input reader failed!\n" << endl;
    exit(1);
  }
  m_pVideoSourceQueue = m_pInput->videoMediaSourceQueue();
  m_paudioSourceQueue = m_pInput->audioMediaSourceQueue();
  if(m_pVideoSourceQueue && m_paudioSourceQueue)
  {
    m_pServer = new CCreateRTPServer(m_pMediaState,m_paudioSourceQueue,
                                     m_pInput->audioCodeId(), m_pVideoSourceQueue,
                                     m_pInput->videoCodeId(), rtpNum, rtspNum, ttl);
  }
  else if (m_pVideoSourceQueue)
  {
    m_pServer = new CCreateRTPServer(m_pMediaState,m_pVideoSourceQueue,
                                     m_pInput->videoCodeId(), rtpNum, rtspNum, ttl);
  }
  else if(m_paudioSourceQueue)
  {
    m_pServer = new CCreateRTPServer(m_pMediaState, m_paudioSourceQueue,
                                     m_pInput->audioCodeId(), rtpNum, rtspNum, ttl);
  }
  if(NULL == m_pServer)
  {
      cout << "Create Server Failed!" << endl;
      exit(1);
  }
}
CServer::~CServer()
{


  if(m_pServer)
  {
    delete m_pServer;
    m_pServer = NULL;
  }
  if(m_pInput)
  {
    delete m_pInput;
    m_pInput = NULL;
  }
  if(m_pMediaState)
  {
    delete m_pMediaState;
    m_pMediaState = NULL;
  }
}

void CServer::start()
{
  m_pMediaState->setState(STATE_RUNNING);

  int ret = pthread_create(&m_pthd, NULL, ThreadFunc, m_pInput);
  if(ret != 0)
  {
    perror("can't create Thread");
    exit(1);
  }

  m_pServer->setRTPAddress("239.255.42.42");
  cout << "Do play" << endl;
  m_pServer->doPlay();

  pthread_join(m_pthd, NULL);
}
void CServer::stop()
{
  m_pServer->stopPlay();
}
