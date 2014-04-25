#ifndef CMYRTSPSERVER_H
#define CMYRTSPSERVER_H
#include "liveMedia.hh"

class CMyRTSPServer:public RTSPServer
{
  public:
    CMyRTSPServer(UsageEnvironment& env,
	     Port ourPort,
	     UserAuthenticationDatabase* authDatabase = NULL,
	     unsigned reclamationTestSeconds = 65)
      :RTSPServer(env,setUpOurSocket(env, ourPort), ourPort, authDatabase, reclamationTestSeconds)
    {

    }
    ~CMyRTSPServer()
    {

    }
};

#endif
