/* 
 * File:   CPacketQueue.h
 * Author: ThinkPad
 *
 * Created on 2012年11月13日, 上午10:06
 */

#ifndef CFRAMEQUEUE_H
#define	CFRAMEQUEUE_H
#include "ffmpeg.h"

class CFrameQueue
{
public:
    CFrameQueue();
    virtual ~CFrameQueue();

    bool Push(AVFrame* pFrame);
    bool Pop(AVFrame* pFrame);
    void ClearQueue();
    AVFrame* Front();
    int  Size();

private:
    queue<AVFrame*> m_Queue;
    pthread_mutex_t m_Mutex;
};

#endif	/* CPACKETQUEUE_H */

