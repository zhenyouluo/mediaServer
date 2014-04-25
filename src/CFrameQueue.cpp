/* 
 * File:   CIQueue.cpp
 * Author: ThinkPad
 * 
 * Created on 2012年11月13日, 上午10:06
 */

#include "CFrameQueue.h"

CFrameQueue::CFrameQueue()
{
  pthread_mutex_init(&m_Mutex, NULL);
}

CFrameQueue::~CFrameQueue()
{
  pthread_mutex_destroy(&m_Mutex);
}

bool CFrameQueue::Push(AVFrame* pFrame)
{
  pthread_mutex_lock(&m_Mutex);
  m_Queue.push(pFrame);
  pthread_mutex_unlock(&m_Mutex);
  return true;
}

bool CFrameQueue::Pop(AVFrame* pFrame)
{
  if (NULL == pFrame)
  {
    return false;
  }

  pthread_mutex_lock(&m_Mutex);
  if (!m_Queue.empty())
  {
    pFrame = m_Queue.front();
    m_Queue.pop();
  }
  pthread_mutex_unlock(&m_Mutex);

  return true;
}

int CFrameQueue::Size()
{
  int nSize = 0;
  pthread_mutex_lock(&m_Mutex);
  nSize = m_Queue.size();
  pthread_mutex_unlock(&m_Mutex);
  return nSize;
}

AVFrame* CFrameQueue::Front()
{
  AVFrame* pFrame = NULL;
  pthread_mutex_lock(&m_Mutex);
  if (!m_Queue.empty())
  {
    pFrame = m_Queue.front();
  }
  pthread_mutex_unlock(&m_Mutex);
  return pFrame;
}

void CFrameQueue::ClearQueue()
{
  AVFrame* pFrame = NULL;
  pthread_mutex_lock(&m_Mutex);
  while(!m_Queue.empty())
  {
    pFrame = m_Queue.front();
    if (NULL != pFrame)
    {
      avcodec_free_frame(&pFrame);
      pFrame = NULL;
      m_Queue.pop();
    }
  }
  pthread_mutex_unlock(&m_Mutex);
}
