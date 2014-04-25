#include "HelpFunc.h"

#include"CLog.h"

#ifdef PRINT_DEBUG_INFO
#include<iostream>
using namespace std;
#endif

void checkPtr(void *ptr, char *str)
{
  if(!ptr)
  {
    if(str)
    {
      LOG(LOG_TYPE_ERROR, str);
#ifdef PRINT_DEBUG_INFO
      cout << str << endl;
#endif
      exit(1);
    }
  }
}
