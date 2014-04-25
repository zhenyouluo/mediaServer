#include <iostream>
#include <string>
using namespace std;

#include<pthread.h>
#include<unistd.h>

#include"CServer.h"

int main(int argc, char* argv[])
{
  string sourceName;
  if(argc == 1)
  {
    sourceName = "/home/panyin/Videos/music.mp3";
  }
  else if(argc == 2)
  {
    sourceName = argv[1];
  }
  else
  {
    cout << "More Input Options!" << endl;
    return 1;
  }

  InitLog();
  //char *inLiveSourceName = "udp://192.168.110.235:1234?multicast=1";

  // string address("224.0.0.2");

  CServer *pServer = new CServer(sourceName,6666);
  if(pServer)
    pServer->start();

  return 0;
}
