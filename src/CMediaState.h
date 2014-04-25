#ifndef CMEDIASTATE_H
#define CMEDIASTATE_H
#include"CSubject.h"
#include<list>
using namespace std;


class CMediaState:public CSubject
{
public:
  CMediaState();
  ~CMediaState();
  void registerObserver(CObserver *o);
  void removeObserver(CObserver *o); 

  void setState(MEDIA_STATE_ENUM state);
private:
  void clearObservers();
  void notifyObservers();
  void stateChanged();

private:
  list<CObserver *> m_observers;
  MEDIA_STATE_ENUM m_state;
};

#endif // CMEDIASTATE_H
