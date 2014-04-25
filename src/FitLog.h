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

#ifndef FITLOG_H           //��ͷ�ļ���ȫ��д�����壬��global.h����ΪGLOBAL_H��
                           //Ӧע������ļ�������
#define FITLOG_H
/* ����β��� */

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


//�����������־�Ƿ�����ǿ�����������ʱ��������λ: ��
#define FLUSH_CHECK_INTERVAL                 5

/*********************************************************************
 *
 * ö�����ƣ�LOG_PRIORITY_t
 * ö�ٹ��ܣ�������־���ȼ�
 * �޸��б���YYYY-MM-DD hh:mm by xxxx)
 * �����˵����
 * ������
 * 
 ***********************************************************************/
//ȡֵԽС�����ȼ�Խ��
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
 * �ṹ���ƣ�ModMap_t
 * �ṹ���ܣ�����ģ�����ƺ�ģ�����ȼ�����Ϣ
 * �޸��б���YYYY-MM-DD hh:mm by xxxx)
 * �����˵����
 * ������
 * 
 ***********************************************************************/
typedef struct _ModInfo_t{
    string sModName;             /*ģ����*/
    int nModId;                  /*ģ���*/
    int nPriority;               /*ģ�����ȼ�*/
} ModInfo_t;

/*********************************************************************
 *
 * �ṹ���ƣ�PeriodLogInfo_t
 * �ṹ���ܣ�������������־��Ϣ
 * �޸��б���YYYY-MM-DD hh:mm by xxxx)
 * �����˵����
 * ������
 * 
 ***********************************************************************/
typedef struct _PeriodLogInfo_t {
    int nPriority;               /*��־���ȼ�*/
    unsigned long lCount;        /*��Ϣ�ظ�����*/
    time_t  lTime;               /*��־�ϴμ�¼ʱ��*/ 
} PeriodLogInfo_t;

/*********************************************************************
 *
 * �����ƣ�CFitLog
 * �๦�ܣ�
 * �޸��б���YYYY-MM-DD hh:mm by xxxx)
 * �����˵����
 * ������
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
    //��m_vectorModInfo�в���ģ����pcModName
    int findModule(const string sModName);

    void flushPeriodLog();

    static CFitLog* m_pFitLog;
        
    //ά��һ��ȫ��Ψһ��ģ���
    int m_nModId;
    
    //��������ע��ģ������ƺ����ȼ�����Ϣ
    std::vector<ModInfo_t> m_vectorModInfo;
    
    log4cpp::Category *m_pRootCategory;
        
    //������ն˵�appender����
    log4cpp::OstreamAppender* m_pCoutAppender;

    //������ڴ��appender����
    log4cpp::StringQueueAppender* m_pMemAppender;
        
    //������ļ���appender����
    log4cpp::FileAppender* m_pFileAppender;

    //������ؾ��ļ���appender����
    log4cpp::RollingFileAppender *m_pRollingFileAppender;

    //���������������־��Ϣ: keyΪ��־��Ϣ
    std::map<string, PeriodLogInfo_t> m_periodLogInfo;

    //��������־ǿ���������־�ļ��е�ʱ��������λ: ��
    int m_nFlushInterval;
    
    //�ϴμ����������־��������ʱ��
    time_t  m_lastChkPeriodLogTime;
		
#if defined (__WIN32__) || defined( _WIN32)
		CRITICAL_SECTION m_mutex;
#else
    pthread_mutex_t m_mutex;
#endif
};

/* ����ν��� */
#endif // #define FITLOG_H
