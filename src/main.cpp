#include <iostream>
#include <string>
using namespace std;

#include<pthread.h>
#include<unistd.h>

#include"CServer.h"

int main(int argc, char* argv[])
{
  string sourceName;
  if(argc == 2)
  {
    sourceName = argv[1];
  }
  else
  {
    cout << "More Input Options!" << endl;
    return 1;
  }

  InitLog();


  CServer *pServer = new CServer(sourceName,6666);
  if(pServer)
    pServer->start();

  return 0;
}
