#include "CMediaState.h"

CMediaState::CMediaState()
{
}
CMediaState::~CMediaState()
{

}

void CMediaState::registerObserver(CObserver *o)
{
  if(o)
    m_observers.insert(m_observers.end(), o);
}
void CMediaState::removeObserver(CObserver *o)
{
  for(list<CObserver*>::iterator iter = m_observers.begin(); iter != m_observers.end(); iter++)
  {
    if(*iter == o)
    {
      m_observers.remove(*iter);
      break;
    }
  }
}
void CMediaState::clearObservers()
{
  m_observers.clear();
}

void CMediaState::notifyObservers()
{
  for(list<CObserver*>::const_iterator cnt_iter = m_observers.begin(); cnt_iter != m_observers.end(); cnt_iter++)
  {
    (*cnt_iter)->update(m_state);
  }
}
void CMediaState::stateChanged()
{
  notifyObservers();
}
void CMediaState::setState(MEDIA_STATE_ENUM state)
{
  if(m_state != state)
  {
    m_state = state;
    stateChanged();
  }
}
