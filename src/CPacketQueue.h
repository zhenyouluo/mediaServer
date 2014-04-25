/* 
 * File:   CPacketQueue.h
 * Author: ThinkPad
 *
 * Created on 2012年11月13日, 上午10:06
 */

#ifndef CPACKETQUEUE_H
#define	CPACKETQUEUE_H
#include "ffmpeg.h"
class CPacketQueue
{
public:
    CPacketQueue();
    virtual ~CPacketQueue();

    bool Push(AVPacket* pPkt);
    bool Pop(AVPacket* pPkt);
    void ClearQueue();
    AVPacket* Front();
    int  Size();

private:
    queue<AVPacket*> m_Queue;
    pthread_mutex_t m_Mutex;
};

#endif	/* CPACKETQUEUE_H */

