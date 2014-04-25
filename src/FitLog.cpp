#if defined (__WIN32__)||(_WIN32)
//nothing
#else
#include <signal.h>
#include <pthread.h>
#endif

/***************************************************************************
*   File Name: FitLog.cpp
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   The above copyright notice and this permission notice shall be
*   included in all copies or substantial portions of the Software.
*
*   You should have received a copy of the Figure-IT License
*   along with this program; if not, write to the
*   ShangHai Figure-IT, Inc.
***************************************************************************/

#include <stdarg.h>
#include <stdlib.h>
#include "FitLog.h"
#include "log4cpp/PatternLayout.hh"

/*****************************************************************************
*
*  当前文件函数说明：
*
****************************************************************************
*  文件修改日期列表： YYYY-MM-DD hh:mm by xxxx
*
****************************************************************************/
/* 代码段部分 */

CFitLog* CFitLog::m_pFitLog = NULL;

/*******************************************************************************
*
* 函数名称：CFitLog
* 函数功能：构造函数
* 参数说明：none
* 返回值说明：none
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
CFitLog::CFitLog()
{
	m_nModId = 0;

	m_pRootCategory = NULL;
	m_pCoutAppender = NULL;
	m_pMemAppender = NULL;
	m_pFileAppender = NULL;
	m_pRollingFileAppender = NULL;

	m_nFlushInterval = 30;
	m_lastChkPeriodLogTime = time(NULL);

#if defined(__WIN32__)||defined(_WIN32)
	InitializeCriticalSection(&m_mutex);
#else
	pthread_mutex_init(&m_mutex, NULL);
#endif
}

/*******************************************************************************
*
* 函数名称：~CFitLog
* 函数功能：析构函数
* 参数说明：none
* 返回值说明：none
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
CFitLog::~CFitLog()
{
#if defined(__WIN32__)||defined(_WIN32)
	DeleteCriticalSection(&m_mutex);
#else
	pthread_mutex_destroy(&m_mutex);
#endif
}

/*******************************************************************************
*
* 函数名称：Instance
* 函数功能：返回CFitLog单实例对象
* 参数说明：none
* 返回值说明：CFitLog对象指针
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
CFitLog* CFitLog::Instance()
{
	if (m_pFitLog == NULL)
	{
		m_pFitLog = new CFitLog();
		if (m_pFitLog == NULL)
		{
			cout << __FUNCTION__ << "alloc mem fail" << endl;
		}
		else
		{
			m_pFitLog->m_pRootCategory = &log4cpp::Category::getRoot();
		}
	}

	return m_pFitLog;
}

/*******************************************************************************
*
* 函数名称：registerMod
* 函数功能：注册一个应用模块
* 参数说明：sModName: 应用模块名称，在记录日志时会使用该名称
* 返回值说明：应用模块id，可以在其他函数(如：CFitLog::log)里使用,
如果失败则返回-1
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
int CFitLog::registerMod(const string sModName)
{
	int nModId;

	/*在m_vectorModInfo里查找sModName是否已经注册*/
	nModId = findModule(sModName);

	/*该模块已经注册*/
	if (nModId != -1)
	{
		cout << "registerMod: " << sModName << "has been registered." << endl;
		return -1;
	}

	/*在m_vectorModInfo里添加一条记录*/
	ModInfo_t stModInfo;
	stModInfo.sModName = sModName;
	stModInfo.nModId = m_nModId++;
	stModInfo.nPriority = LOG_TYPE_DEBUG;
	m_vectorModInfo.push_back(stModInfo);

	return stModInfo.nModId;
}

/*******************************************************************************
*
* 函数名称：findModule
* 函数功能：查找模块是否已经注册
* 参数说明：sModName: 应用模块名称，在记录日志时会使用该名称
* 返回值说明：应用模块id，如果没有找到则返回-1
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
int CFitLog::findModule(const string sModName)
{
	for (unsigned int i = 0; i < m_vectorModInfo.size(); i++)
	{
		if (m_vectorModInfo[i].sModName == sModName)
		{
			return i;
		}
	}

	/*没有找到*/
	return -1;
}

/*******************************************************************************
*
* 函数名称：log
* 函数功能：记录一条日志信息
* 参数说明：iModId: 模块号，模块号获取请参见registerMod函数。
nPriority: 日志优先级，具体定义值请参考LOG_PRIORITY_t
pcFormat: 格式串
* 返回值说明：无
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
void CFitLog::log(int iModId, int nPriority, const char* pcFormat, ...)
{
	va_list pArg;
	va_start(pArg, pcFormat);

	vlog(iModId, nPriority, pcFormat, pArg);

	va_end(pArg);
}

/*******************************************************************************
*
* 函数名称：vlog
* 函数功能：记录一条日志信息
* 参数说明：iModId: 模块号，模块号获取请参见registerMod函数。
nPriority: 日志优先级，具体定义值请参考LOG_PRIORITY_t
pcFormat: 格式串
pArg: 参数列表
* 返回值说明：无
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
void CFitLog::vlog(int iModId, int nPriority, const char* pcFormat, va_list pArg)
{
	/*检查iModId参数值是否合法*/
	if ((iModId < 0) || (iModId >= m_nModId))
	{
		cout << "log: invalid modId " << iModId << endl;
		return;
	}

	/*根据iModId在m_vectorModInfo中找到该模块对应的记录*/
	ModInfo_t &stModInfo = m_vectorModInfo[iModId];

	/*优先级太低,不记录*/
	if (nPriority > stModInfo.nPriority)
	{
		return;
	}

	char *pcText = NULL;
	int nResult;

	/*生成格式化后的字符串*/
	nResult = vasprintf(&pcText, pcFormat, pArg);
	//nResult = sprintf(pcText, pcFormat, pArg);

	//格式化字符串出错
	if (nResult == -1)
	{
		cout << __FUNCTION__ << "call vasprintf fail" << endl;
		return;
	}

	/*调用log4cpp的log函数记录日志*/
	//pthread_mutex_lock(&m_mutex);
	Lock();
	m_pRootCategory->log(nPriority, string(pcText));

	/*free vasprintf函数分配的内存*/
	free(pcText);

	//must be locked......
	flushPeriodLog();

	Unlock();
	//pthread_mutex_unlock(&m_mutex);
}

/*******************************************************************************
*
* 函数名称：logPeriod
* 函数功能：记录周期性日志信息，防止日志文件过大
* 参数说明：iModId: 模块号，模块号获取请参见registerMod函数。
nPriority: 日志优先级，具体定义值请参考LOG_PRIORITY_t
pcFormat: 格式串
* 返回值说明：无
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
void CFitLog::logPeriod(int iModId, int nPriority, const char* pcFormat, ...)
{
	va_list pArg;

	va_start(pArg, pcFormat);
	vlogPeriod(iModId, nPriority, pcFormat, pArg);
	va_end(pArg);
}

/*******************************************************************************
*
* 函数名称：vlogPeriod
* 函数功能：记录周期性日志信息，防止日志文件过大
* 参数说明：iModId: 模块号，模块号获取请参见registerMod函数。
nPriority: 日志优先级，具体定义值请参考LOG_PRIORITY_t
pcFormat: 格式串
pArg: 参数列表
* 返回值说明：无
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
void CFitLog::vlogPeriod(int iModId, int nPriority, const char* pcFormat, va_list pArg)
{
	/*检查iModId参数值是否合法*/
	if ((iModId < 0) || (iModId >= m_nModId))
	{
		cout << "log: invalid modId " << iModId << endl;
		return;
	}

	/*根据iModId在m_vectorModInfo中找到该模块对应的记录*/
	ModInfo_t &stModInfo = m_vectorModInfo[iModId];

	/*优先级太低,不记录*/
	if (nPriority > stModInfo.nPriority)
	{
		return;
	}

	char *pcText = NULL;
	int nResult;

	/*生成格式化后的字符串*/
	nResult = vasprintf(&pcText, pcFormat, pArg);
	//nResult = sprintf(pcText, pcFormat, pArg);

	//格式化字符串出错
	if (nResult == -1)
	{
		cout << __FUNCTION__ << "call vasprintf fail" << endl;
		return;
	}

	string sText(pcText);

	/*释放vasprintf函数分配的内存*/
	free(pcText);

	//pthread_mutex_lock(&m_mutex);

	Lock();
	//查找该日志信息是否已经记录过
	std::map<string, PeriodLogInfo_t>::iterator iterator;
	iterator = m_periodLogInfo.find(sText);

	//首次出现
	if (iterator == m_periodLogInfo.end())
	{
		PeriodLogInfo_t stLogInfo;

		stLogInfo.nPriority = nPriority;
		stLogInfo.lCount = 1;
		stLogInfo.lTime = time(NULL);
		m_periodLogInfo[sText] = stLogInfo;
	}
	else
	{
		PeriodLogInfo_t &stLogInfo = m_periodLogInfo[sText];

		//如果日志之前已经被输出，则需要记录首次出现时间
		if (stLogInfo.lCount == 0)
		{
			stLogInfo.lTime = time(NULL);
		}
		stLogInfo.lCount++;
	}

	flushPeriodLog();
	Unlock();
	// pthread_mutex_unlock(&m_mutex);
}

/*******************************************************************************
*
* 函数名称：setModPriority
* 函数功能：设置模块日志记录优先级,只有大于等于该优先级的信息才会被输出
* 参数说明：iModId: 模块号，模块号获取请参见registerMod函数
nPriority: 日志优先级，具体定义值请参考LOG_PRIORITY_t
* 返回值说明：0-表示执行成功; -1-表示执行失败
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
int CFitLog::setModPriority(int iModId, int nPriority)
{
	/*检查iModId参数值是否合法*/
	if ((iModId < 0) || (iModId >= m_nModId))
	{
		cout << "setModPriority: invalid modId " << iModId << endl;
		return -1;
	}

	/*根据iModId在m_vectorModInfo中找到该模块对应的记录*/
	ModInfo_t &stModInfo = m_vectorModInfo[iModId];

	/*更新该模块的优先级为nPriority*/
	stModInfo.nPriority = nPriority;

	return 0;
}

/*******************************************************************************
*
* 函数名称：setModPriority
* 函数功能：设置模块日志记录优先级,只有大于等于该优先级的信息才会被输出
* 参数说明：sModName: 模块名
nPriority: 日志优先级，具体定义值请参考LOG_PRIORITY_t
* 返回值说明：0-表示执行成功; -1-表示执行失败
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
int CFitLog::setModPriority(const string sModName, int nPriority)
{
	/*在m_vectorModInfo里查找pcModName是否已经注册*/
	int nModId;
	nModId = findModule(sModName);

	/*该模块没有注册*/
	if (nModId == -1)
	{
		cout << "setModPriority: " << sModName << " has not registered." << endl;
		return -1;
	}

	/*更新该模块的优先级为nPriority*/
	m_vectorModInfo[nModId].nPriority = nPriority;

	return 0;
}

/*******************************************************************************
*
* 函数名称：getModPriority
* 函数功能：获取模块日志记录优先级
* 参数说明：sModName: 模块名
nPriority[out]: 模块的日志记录优先级
* 返回值说明：0-表示执行成功; -1-表示执行失败
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
int CFitLog::getModPriority(const string sModName, int& nPriority)
{
	/*在m_vectorModInfo里查找pcModName是否已经注册*/

	int nModId;
	nModId = findModule(sModName);

	/*该模块没有注册*/
	if (nModId == -1)
	{
		cout << "getModPriority: " << sModName << " has not registered." << endl;
		return -1;
	}

	nPriority = m_vectorModInfo[nModId].nPriority;
	return 0;
}

/*******************************************************************************
*
* 函数名称：getModPriority
* 函数功能：获取模块日志记录优先级
* 参数说明：iModId: 模块号，模块号获取请参见registerMod函数
nPriority[out]: 模块的日志记录优先级
* 返回值说明：0-表示执行成功; -1-表示执行失败
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
int CFitLog::getModPriority(int iModId, int& nPriority)
{
	/*检查iModId参数值是否合法*/
	if ((iModId < 0) || (iModId >= m_nModId))
	{
		cout << "getModPriority: invalid modId " << iModId << endl;
		return -1;
	}

	nPriority = m_vectorModInfo[iModId].nPriority;

	return 0;
}

/*******************************************************************************
*
* 函数名称：setAllPriority
* 函数功能：设置所有模块的日志记录优先级,只有大于等于该优先级的信息才会被输出
* 参数说明：nPriority: 日志优先级，具体定义值请参考LOG_PRIORITY_t
* 返回值说明：0-表示执行成功; -1-表示执行失败
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
int CFitLog::setAllPriority(int nPriority)
{
	unsigned int i;

	/*遍历m_vectorModInfo*/
	for (i = 0; i < m_vectorModInfo.size(); i++)
	{
		/*更新模块的优先级为nPriority*/
		m_vectorModInfo[i].nPriority = nPriority;
	}

	return 0;
}


/*******************************************************************************
*
* 函数名称：getLayout
* 函数功能：创建一个PatternLayout
* 参数说明：无
* 返回值说明：Layout对象指针
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
log4cpp::Layout* CFitLog::getLayout()
{
	log4cpp::PatternLayout* pLayout;

	pLayout = new log4cpp::PatternLayout();
	if (pLayout == NULL)
	{
		cout << __FUNCTION__ << "alloc mem fail" << endl;
	}
	else
	{
		pLayout->setConversionPattern("%d: %p %c %x: %m%n");
	}

	return pLayout;
}

/*******************************************************************************
*
* 函数名称：enableStdout
* 函数功能：使能日志输出到终端
* 参数说明：无
* 返回值说明：0-表示执行成功; -1-表示执行失败
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
int CFitLog::enableStdout(void)
{
	/*已经使能日志输出到终端*/
	if (m_pCoutAppender != NULL)
	{
		cout << "std output is enabled already." << endl;
		return 0;
	}

	/*创建log4cpp::OstreamAppender对象*/
	m_pCoutAppender = new log4cpp::OstreamAppender("OstreamAppender", &cout);
	if (m_pCoutAppender == NULL)
	{
		cout << __FUNCTION__ << "alloc mem fail" << endl;
		return -1;
	}

	/*创建一个layout*/
	log4cpp::Layout* pLayout = getLayout();

	/*设置m_pCoutAppender的Layout*/
	m_pCoutAppender->setLayout(pLayout);

	/*将m_pCoutAppender添加到m_pRootCategory中*/
	m_pRootCategory->addAppender(m_pCoutAppender);

	cout << "enable std output success" << endl;

	return 0;
}

/*******************************************************************************
*
* 函数名称：disableStdOutput
* 函数功能：禁止日志输出到终端
* 参数说明：无
* 返回值说明：0-表示执行成功; -1-表示执行失败
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
int CFitLog::disableStdOutput(void)
{
	/*还没有使能日志输出到终端*/
	if (m_pCoutAppender == NULL)
	{
		cout << "std output is disabled." << endl;
		return 0;
	}

	/*将m_pCoutAppender从m_pRootCategory中删除*/
	m_pRootCategory->removeAppender(m_pCoutAppender);

	delete m_pCoutAppender;
	m_pCoutAppender = NULL;

	cout << "disable std output success" << endl;

	return 0;
}

/*******************************************************************************
*
* 函数名称：enableMemOutput
* 函数功能：使能日志输出到内存
* 参数说明：无
* 返回值说明：0-表示执行成功; -1-表示执行失败
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
int CFitLog::enableMemOutput(void)
{
	/*已经使能日志输出到内存*/
	if (m_pMemAppender != NULL)
	{
		cout << "memory output is enabled already." << endl;
		return 0;
	}

	/*创建log4cpp::StringQueueAppender对象*/
	m_pMemAppender = new log4cpp::StringQueueAppender("StringQueueAppender");
	if (m_pMemAppender == NULL)
	{
		cout << __FUNCTION__ << "alloc mem fail" << endl;
		return -1;
	}

	/*创建一个layout*/
	log4cpp::Layout* pLayout = getLayout();

	/*设置m_pMemAppender的Layout*/
	m_pMemAppender->setLayout(pLayout);

	/*将m_pMemAppender添加到m_pRootCategory中*/
	m_pRootCategory->addAppender(m_pMemAppender);

	cout << "enable memory output success" << endl;

	return 0;
}

/*******************************************************************************
*
* 函数名称：disableMemOutput
* 函数功能：禁止日志输出到内存
* 参数说明：无
* 返回值说明：0-表示执行成功; -1-表示执行失败
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
int CFitLog::disableMemOutput(void)
{
	/*还没有使能日志输出到内存*/
	if (m_pMemAppender == NULL)
	{
		cout << "mem output is disabled already." << endl;
		return 0;
	}

	/*将m_pMemAppender从m_pRootCategory中删除*/
	m_pRootCategory->removeAppender(m_pMemAppender);

	delete m_pMemAppender;
	m_pMemAppender = NULL;

	cout << "disable memory output success" << endl;

	return 0;
}

/*******************************************************************************
*
* 函数名称：getMemLog
* 函数功能：获取保存在内存中的日志信息
* 参数说明：strQuque[out]: 获取到的日志信息列表
* 返回值说明：0-表示执行成功; -1-表示执行失败
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
int CFitLog::getMemLog(queue<string>& strQuque)
{
	/*还没有使能日志输出到内存*/
	if (m_pMemAppender == NULL)
	{
		cout << "mem output is disabled now." << endl;
		return -1;
	}

	strQuque = m_pMemAppender->getQueue();
	return 0;
}

/*******************************************************************************
*
* 函数名称：enableFileOutput
* 函数功能：使能日志输出到文件
* 参数说明：pcFileName: 日志文件名
* 返回值说明：0-表示执行成功; -1-表示执行失败
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
int CFitLog::enableFileOutput(string strFileName)
{
	/*已经使能日志输出到文件*/
	if (m_pFileAppender != NULL)
	{
		cout << "file output is enabled already." << endl;
		return 0;
	}

	/*创建log4cpp::FileAppender对象*/
	m_pFileAppender = new log4cpp::FileAppender("FileAppender", strFileName);
	if (m_pFileAppender == NULL)
	{
		cout << __FUNCTION__ << "alloc mem fail" << endl;
		return -1;
	}

	/*创建一个layout*/
	log4cpp::Layout* pLayout = getLayout();

	/*设置m_pFileAppender的Layout*/
	m_pFileAppender->setLayout(pLayout);

	/*将m_pFileAppender添加到m_pRootCategory中*/
	m_pRootCategory->addAppender(m_pFileAppender);

	cout << "enable file output success" << endl;

	return 0;
}

/*******************************************************************************
*
* 函数名称：disableFileOutput
* 函数功能：禁止日志输出到文件
* 参数说明：
* 返回值说明：0-表示执行成功; -1-表示执行失败
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
int CFitLog::disableFileOutput(void)
{
	/*还没有使能日志输出到文件*/
	if (m_pFileAppender == NULL)
	{
		cout << "file output is disabled already." << endl;
		return 0;
	}

	/*将m_pFileAppender从m_pRootCategory中删除*/
	m_pRootCategory->removeAppender(m_pFileAppender);

	delete m_pFileAppender;
	m_pFileAppender = NULL;

	cout << "disable file output success" << endl;
	return 0;
}

/*******************************************************************************
*
* 函数名称：enableRollingFileOutput
* 函数功能：使能日志输出到回卷文件
* 参数说明：pcFileName: 日志文件名
maxFileSize: 日志文件最大值, 单位: Byte
uMaxBackupIndex: 日志文件最大备份个数
* 返回值说明：0-表示执行成功; -1-表示执行失败
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
int CFitLog::enableRollingFileOutput(string strFileName,
																		 size_t maxFileSize, unsigned int uMaxBackupIndex)
{
	/*已经使能日志输出到回卷文件*/
	if (m_pRollingFileAppender != NULL)
	{
		cout << "rolling file output is enabled already." << endl;
		return 0;
	}
	
	/*创建log4cpp::RollingFileAppender对象*/
	m_pRollingFileAppender = new log4cpp::RollingFileAppender("RollingFileAppender",strFileName, maxFileSize, uMaxBackupIndex);
	if (m_pRollingFileAppender == NULL)
	{
		cout << __FUNCTION__ << "alloc mem fail" << endl;
		return -1;
	}

	/*创建一个layout*/
	log4cpp::Layout* pLayout = getLayout();

	/*设置m_pRollingFileAppender的Layout*/
	m_pRollingFileAppender->setLayout(pLayout);

	/*将m_pRollingFileAppender添加到m_pRootCategory中*/
	m_pRootCategory->addAppender(m_pRollingFileAppender);

	cout << "enable rolling file output success" << endl;
	return 0;
}

/*******************************************************************************
*
* 函数名称：disableRollingFileOutput
* 函数功能：禁止日志输出到回卷文件
* 参数说明：
* 返回值说明：0-表示执行成功; -1-表示执行失败
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
int CFitLog::disableRollingFileOutput(void)
{
	/*还没有使能日志输出到回卷文件*/
	if (m_pRollingFileAppender == NULL)
	{
		cout << "rolling file output is disabled already." << endl;
		return 0;
	}

	/*将m_pRollingFileAppender从m_pRootCategory中删除*/
	m_pRootCategory->removeAppender(m_pRollingFileAppender);

	delete m_pRollingFileAppender;
	m_pRollingFileAppender = NULL;

	cout << "disable rolling file output success" << endl;
	return 0;
}

/*******************************************************************************
*
* 函数名称：setPeriodLogFlushInterval
* 函数功能：设置周期性日志强制输出到日志文件中的时间间隔
* 参数说明：nSeconds: 时间间隔, 单位: 秒
* 返回值说明：None
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
void CFitLog::setPeriodLogFlushInterval(int nSeconds)
{
	m_nFlushInterval = nSeconds;
}

/*******************************************************************************
*
* 函数名称：flushPeriodLog
* 函数功能：将保存在内存中的周期性日志输出到日志文件中
* 参数说明：
* 返回值说明：None
* 修改列表：（YYYY-MM-DD hh:mm by xxxx)
* 相关性说明：
* 其他：
*
******************************************************************************/
void CFitLog::flushPeriodLog()
{
	time_t lTime = time(NULL);

	//刷新时间未到
	if (lTime - m_lastChkPeriodLogTime < FLUSH_CHECK_INTERVAL)
	{
		return;
	}
	m_lastChkPeriodLogTime = lTime;

	std::map<string, PeriodLogInfo_t>::iterator iterator;

	//遍历所有的周期性日志信息
	for (iterator = m_periodLogInfo.begin(); iterator != m_periodLogInfo.end(); iterator++)
	{
		PeriodLogInfo_t &stLogInfo = iterator->second;

		//日志满足强制输出时间间隔
		if ((stLogInfo.lCount > 0) && (lTime - stLogInfo.lTime > m_nFlushInterval))
		{
			//输出日志信息
			string sText(iterator->first);

			//日志后添加重复计数值
			char acBuf[100];
#if defined(__WIN32__)||defined (_WIN32)
			_snprintf_s(acBuf, sizeof(acBuf), "%ld", stLogInfo.lCount);
#else
			snprintf(acBuf, sizeof(acBuf), "%ld", stLogInfo.lCount);
#endif

			sText += " RepeatCount=";
			sText += acBuf;

			m_pRootCategory->log(stLogInfo.nPriority, sText);

			//stLogInfo.lCount = 0;
			m_periodLogInfo.erase(iterator);
		}
	}
}

void CFitLog::Lock()
{
#if defined(__WIN32__)||defined(_WIN32)
	EnterCriticalSection(&m_mutex);
#else
	pthread_mutex_lock(&m_mutex);
#endif
}
void CFitLog::Unlock()
{
#if defined(__WIN32__)||defined(_WIN32)
	LeaveCriticalSection(&m_mutex);
#else
	pthread_mutex_unlock(&m_mutex);
#endif
}
/* 代码段结束 */

