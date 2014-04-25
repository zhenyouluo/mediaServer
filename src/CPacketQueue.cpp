/* 
 * File:   CIQueue.cpp
 * Author: ThinkPad
 * 
 * Created on 2012年11月13日, 上午10:06
 */

#include "CPacketQueue.h"

CPacketQueue::CPacketQueue()
{
    pthread_mutex_init(&m_Mutex, NULL);
}

CPacketQueue::~CPacketQueue()
{
    pthread_mutex_destroy(&m_Mutex);
}

bool CPacketQueue::Push(AVPacket* pPkt)
{
    AVPacket* pPkt1 = NULL;

    if (NULL != pPkt)
    {
        if (av_dup_packet(pPkt) < 0)
        {
            return false;
        }

        pPkt1 = (AVPacket*) av_malloc(sizeof (AVPacket));
        if (NULL == pPkt1)
        {
            return false;
        }
        *pPkt1 = *pPkt;
    }
    else
    {
        return false;
    }
    pthread_mutex_lock(&m_Mutex);
    m_Queue.push(pPkt1);
    pthread_mutex_unlock(&m_Mutex);
    return true;
}

bool CPacketQueue::Pop(AVPacket* pPkt)
{
    if (NULL == pPkt)
    {
        return false;
    }

    AVPacket* pPkt1 = NULL;
    pthread_mutex_lock(&m_Mutex);
    if (!m_Queue.empty())
    {
        pPkt1 = m_Queue.front();
        *pPkt = *pPkt1;
        av_free(pPkt1);
        m_Queue.pop();
    }
    pthread_mutex_unlock(&m_Mutex);

    if (NULL == pPkt1)
    {
        return false;
    }
    else
    {
        return true;
    }
}

int CPacketQueue::Size()
{
    int nSize = 0;
    pthread_mutex_lock(&m_Mutex);
    nSize = m_Queue.size();
    pthread_mutex_unlock(&m_Mutex);
    return nSize;
}

AVPacket* CPacketQueue::Front()
{
    AVPacket* pPkt = NULL;
    pthread_mutex_lock(&m_Mutex);
    if (!m_Queue.empty())
    {
        pPkt = m_Queue.front();
    }
    pthread_mutex_unlock(&m_Mutex);
    return pPkt;
}

void CPacketQueue::ClearQueue()
{
    AVPacket * pPkt = NULL;
    pthread_mutex_lock(&m_Mutex);
    while(!m_Queue.empty())
    {
        pPkt = m_Queue.front();
        if (NULL != pPkt)
        {
            av_free(pPkt);
            pPkt = NULL;
            m_Queue.pop();
        }
    }
    pthread_mutex_unlock(&m_Mutex);
}
