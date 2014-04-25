#ifndef CSERVER_H
#define CSERVER_H

#include"CCreateRTPServer.h"
#include"CReadPacket.h"
#include"CMediaState.h"
#include<string>
using namespace std;

#include<pthread.h>

class CServer
{
public:
  explicit CServer(const string &inputFile, unsigned short rtpNum, unsigned short rtspNum = 8554,
                   unsigned char ttl = 255);
  ~CServer();
  void start();
  void stop();
private:
  pthread_t m_pthd ;
  CCreateRTPServer *m_pServer;
  CReadPacket *m_pInput;

  CPacketQueue *m_pVideoSourceQueue;
  CPacketQueue *m_paudioSourceQueue;
  CMediaState *m_pMediaState;
};

#endif // CSERVER_H
