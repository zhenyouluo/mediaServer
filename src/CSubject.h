#ifndef CSUBJECT_H
#define CSUBJECT_H
#include"CObserver.h"


class CSubject
{
public:
  CSubject();
  virtual ~CSubject();
  virtual void registerObserver(CObserver *o)=0;
  virtual void removeObserver(CObserver *o)=0;
  virtual void clearObservers()=0;
  virtual void notifyObservers()=0;
};

#endif // SUBJECT_H
