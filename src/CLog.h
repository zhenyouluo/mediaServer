#ifndef CLOG_H
#define CLOG_H

#include "FitLog.h"

// ȫ�ֱ���
extern int g_nModId;

// ȫ�ֺ���
void LOG(LOG_PRIORITY_t enmPriority, const char* pcFormat...);
void LogPeriod(LOG_PRIORITY_t enmPriority, const char* pcFormat...);

void InitLog();



#endif
