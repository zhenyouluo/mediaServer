/***************************************************************************
 *   File Name: FitLog.hpp
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

#ifndef FITLOG_H           //用头文件名全大写来定义，如global.h定义为GLOBAL_H。
                           //应注意避免文件重名。
#define FITLOG_H
/* 代码段部分 */

#include <cstdio>
#include <cstdarg>
#include <cstdlib>

#include <iostream>
#include <sstream>
#include <string>
#include <queue>
#include <vector>
#include <time.h>

#include "log4cpp/Priority.hh"
#include "log4cpp/Category.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/StringQueueAppender.hh"
#include "log4cpp/RollingFileAppender.hh"

#include "log4cpp/FileAppender.hh"
#include "log4cpp/BasicLayout.hh"

using namespace std;

#if defined(__WIN32__) || defined(_WIN32)
#include <process.h>
#include <Windows.h>
#else
#include <pthread.h>
#endif


//检查周期性日志是否满足强制输出条件的时间间隔，单位: 秒
#define FLUSH_CHECK_INTERVAL                 5

/*********************************************************************
 *
 * 枚举名称：LOG_PRIORITY_t
 * 枚举功能：定义日志优先级
 * 修改列表：（YYYY-MM-DD hh:mm by xxxx)
 * 相关性说明：
 * 其他：
 * 
 ***********************************************************************/
//取值越小，优先级越高
typedef enum 
{
    LOG_TYPE_DEBUG = log4cpp::Priority::DEBUG, 
    LOG_TYPE_INFO = log4cpp::Priority::INFO,
    LOG_TYPE_NOTICE = log4cpp::Priority::NOTICE,
    LOG_TYPE_WARN = log4cpp::Priority::WARN, 
    LOG_TYPE_ERROR = log4cpp::Priority::ERROR,
    LOG_TYPE_FATAL = log4cpp::Priority::FATAL
} LOG_PRIORITY_t;

/*********************************************************************
 *
 * 结构名称：ModMap_t
 * 结构功能：保存模块名称和模块优先级等信息
 * 修改列表：（YYYY-MM-DD hh:mm by xxxx)
 * 相关性说明：
 * 其他：
 * 
 ***********************************************************************/
typedef struct _ModInfo_t{
    string sModName;             /*模块名*/
    int nModId;                  /*模块号*/
    int nPriority;               /*模块优先级*/
} ModInfo_t;

/*********************************************************************
 *
 * 结构名称：PeriodLogInfo_t
 * 结构功能：保存周期性日志信息
 * 修改列表：（YYYY-MM-DD hh:mm by xxxx)
 * 相关性说明：
 * 其他：
 * 
 ***********************************************************************/
typedef struct _PeriodLogInfo_t {
    int nPriority;               /*日志优先级*/
    unsigned long lCount;        /*消息重复次数*/
    time_t  lTime;               /*日志上次记录时间*/ 
} PeriodLogInfo_t;

/*********************************************************************
 *
 * 类名称：CFitLog
 * 类功能：
 * 修改列表：（YYYY-MM-DD hh:mm by xxxx)
 * 相关性说明：
 * 其他：
 * 
 ***********************************************************************/
class CFitLog 
{

public:
    ~CFitLog();

    static CFitLog* Instance();
    
    int registerMod(const string sModName);
    
    void log(int iModId, int nPriority, const char* pcFormat, ...);

    void vlog(int iModId, int nPriority, const char* pcFormat, va_list pArg);

    void logPeriod(int iModId, int nPriority, const char* pcFormat, ...);

    void vlogPeriod(int iModId, int nPriority, const char* pcFormat, va_list pArg);

    void setPeriodLogFlushInterval(int nSeconds);

    int setModPriority(int iModId, int nPriority);

    int setModPriority(const string sModName, int nPriority);

    int getModPriority(const string sModName, int& nPriority);

    int getModPriority(int iModId, int& nPriority);
    
    int setAllPriority(int enmPriority);
    
    int enableStdout(void);
    
    int disableStdOutput(void);    
    
    int enableMemOutput(void);

    int disableMemOutput(void);

    int getMemLog(queue<string>& strQuque);
        
    int enableFileOutput(string strFileName);    
    
    int disableFileOutput(void);    

    int enableRollingFileOutput(string strFileName, size_t maxFileSize, unsigned int uMaxBackupIndex=1);    
    
    int disableRollingFileOutput(void); 

protected:
    CFitLog();

    log4cpp::Layout* getLayout();

private:
	void Lock();
	void Unlock();

private:
    //在m_vectorModInfo中查找模块名pcModName
    int findModule(const string sModName);

    void flushPeriodLog();

    static CFitLog* m_pFitLog;
        
    //维护一个全局唯一的模块号
    int m_nModId;
    
    //保存所有注册模块的名称和优先级等信息
    std::vector<ModInfo_t> m_vectorModInfo;
    
    log4cpp::Category *m_pRootCategory;
        
    //输出到终端的appender对象
    log4cpp::OstreamAppender* m_pCoutAppender;

    //输出到内存的appender对象
    log4cpp::StringQueueAppender* m_pMemAppender;
        
    //输出到文件的appender对象
    log4cpp::FileAppender* m_pFileAppender;

    //输出到回卷文件的appender对象
    log4cpp::RollingFileAppender *m_pRollingFileAppender;

    //保存周期性输出日志信息: key为日志信息
    std::map<string, PeriodLogInfo_t> m_periodLogInfo;

    //周期性日志强制输出到日志文件中的时间间隔，单位: 秒
    int m_nFlushInterval;
    
    //上次检查周期性日志缓冲区的时间
    time_t  m_lastChkPeriodLogTime;
		
#if defined (__WIN32__) || defined( _WIN32)
		CRITICAL_SECTION m_mutex;
#else
    pthread_mutex_t m_mutex;
#endif
};

/* 代码段结束 */
#endif // #define FITLOG_H
