#ifndef COBSERVER_H
#define COBSERVER_H

typedef enum{
  STATE_INPUT_END,
  STATE_STOP,
  STATE_RUNNING
}MEDIA_STATE_ENUM;

class CObserver
{
public:
  CObserver();
  virtual ~CObserver();
  virtual void update(MEDIA_STATE_ENUM state)=0;
};

#endif // OBSERVER_H
