#include "CLog.h"

int g_nModId;

void InitLog()
{
	g_nModId = CFitLog::Instance()->registerMod("Live555Server");
	CFitLog::Instance()->setModPriority(g_nModId, LOG_TYPE_DEBUG);
	string logFileName("Live555Server.log");
	
	CFitLog::Instance()->enableRollingFileOutput(logFileName, 100*1024*1024, 2);
	return;
}
void LOG(LOG_PRIORITY_t enmPriority, const char* pcFormat...)
{
	va_list pArg;

	va_start(pArg, pcFormat);
	CFitLog::Instance()->vlog(g_nModId, enmPriority, pcFormat, pArg);
	va_end(pArg);
}

void LogPeriod(LOG_PRIORITY_t enmPriority, const char* pcFormat...)
{
	va_list pArg;

	va_start(pArg, pcFormat);
	CFitLog::Instance()->vlogPeriod(g_nModId, enmPriority, pcFormat, pArg);
	va_end(pArg);
}
