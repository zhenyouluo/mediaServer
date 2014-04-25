#ifndef CLOG_H
#define CLOG_H

#include "FitLog.h"

// 全局变量
extern int g_nModId;

// 全局函数
void LOG(LOG_PRIORITY_t enmPriority, const char* pcFormat...);
void LogPeriod(LOG_PRIORITY_t enmPriority, const char* pcFormat...);

void InitLog();



#endif
