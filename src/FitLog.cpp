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
*  ��ǰ�ļ�����˵����
*
****************************************************************************
*  �ļ��޸������б� YYYY-MM-DD hh:mm by xxxx
*
****************************************************************************/
/* ����β��� */

CFitLog* CFitLog::m_pFitLog = NULL;

/*******************************************************************************
*
* �������ƣ�CFitLog
* �������ܣ����캯��
* ����˵����none
* ����ֵ˵����none
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
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
* �������ƣ�~CFitLog
* �������ܣ���������
* ����˵����none
* ����ֵ˵����none
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
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
* �������ƣ�Instance
* �������ܣ�����CFitLog��ʵ������
* ����˵����none
* ����ֵ˵����CFitLog����ָ��
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
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
* �������ƣ�registerMod
* �������ܣ�ע��һ��Ӧ��ģ��
* ����˵����sModName: Ӧ��ģ�����ƣ��ڼ�¼��־ʱ��ʹ�ø�����
* ����ֵ˵����Ӧ��ģ��id����������������(�磺CFitLog::log)��ʹ��,
���ʧ���򷵻�-1
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
int CFitLog::registerMod(const string sModName)
{
	int nModId;

	/*��m_vectorModInfo�����sModName�Ƿ��Ѿ�ע��*/
	nModId = findModule(sModName);

	/*��ģ���Ѿ�ע��*/
	if (nModId != -1)
	{
		cout << "registerMod: " << sModName << "has been registered." << endl;
		return -1;
	}

	/*��m_vectorModInfo�����һ����¼*/
	ModInfo_t stModInfo;
	stModInfo.sModName = sModName;
	stModInfo.nModId = m_nModId++;
	stModInfo.nPriority = LOG_TYPE_DEBUG;
	m_vectorModInfo.push_back(stModInfo);

	return stModInfo.nModId;
}

/*******************************************************************************
*
* �������ƣ�findModule
* �������ܣ�����ģ���Ƿ��Ѿ�ע��
* ����˵����sModName: Ӧ��ģ�����ƣ��ڼ�¼��־ʱ��ʹ�ø�����
* ����ֵ˵����Ӧ��ģ��id�����û���ҵ��򷵻�-1
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
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

	/*û���ҵ�*/
	return -1;
}

/*******************************************************************************
*
* �������ƣ�log
* �������ܣ���¼һ����־��Ϣ
* ����˵����iModId: ģ��ţ�ģ��Ż�ȡ��μ�registerMod������
nPriority: ��־���ȼ������嶨��ֵ��ο�LOG_PRIORITY_t
pcFormat: ��ʽ��
* ����ֵ˵������
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
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
* �������ƣ�vlog
* �������ܣ���¼һ����־��Ϣ
* ����˵����iModId: ģ��ţ�ģ��Ż�ȡ��μ�registerMod������
nPriority: ��־���ȼ������嶨��ֵ��ο�LOG_PRIORITY_t
pcFormat: ��ʽ��
pArg: �����б�
* ����ֵ˵������
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
void CFitLog::vlog(int iModId, int nPriority, const char* pcFormat, va_list pArg)
{
	/*���iModId����ֵ�Ƿ�Ϸ�*/
	if ((iModId < 0) || (iModId >= m_nModId))
	{
		cout << "log: invalid modId " << iModId << endl;
		return;
	}

	/*����iModId��m_vectorModInfo���ҵ���ģ���Ӧ�ļ�¼*/
	ModInfo_t &stModInfo = m_vectorModInfo[iModId];

	/*���ȼ�̫��,����¼*/
	if (nPriority > stModInfo.nPriority)
	{
		return;
	}

	char *pcText = NULL;
	int nResult;

	/*���ɸ�ʽ������ַ���*/
	nResult = vasprintf(&pcText, pcFormat, pArg);
	//nResult = sprintf(pcText, pcFormat, pArg);

	//��ʽ���ַ�������
	if (nResult == -1)
	{
		cout << __FUNCTION__ << "call vasprintf fail" << endl;
		return;
	}

	/*����log4cpp��log������¼��־*/
	//pthread_mutex_lock(&m_mutex);
	Lock();
	m_pRootCategory->log(nPriority, string(pcText));

	/*free vasprintf����������ڴ�*/
	free(pcText);

	//must be locked......
	flushPeriodLog();

	Unlock();
	//pthread_mutex_unlock(&m_mutex);
}

/*******************************************************************************
*
* �������ƣ�logPeriod
* �������ܣ���¼��������־��Ϣ����ֹ��־�ļ�����
* ����˵����iModId: ģ��ţ�ģ��Ż�ȡ��μ�registerMod������
nPriority: ��־���ȼ������嶨��ֵ��ο�LOG_PRIORITY_t
pcFormat: ��ʽ��
* ����ֵ˵������
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
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
* �������ƣ�vlogPeriod
* �������ܣ���¼��������־��Ϣ����ֹ��־�ļ�����
* ����˵����iModId: ģ��ţ�ģ��Ż�ȡ��μ�registerMod������
nPriority: ��־���ȼ������嶨��ֵ��ο�LOG_PRIORITY_t
pcFormat: ��ʽ��
pArg: �����б�
* ����ֵ˵������
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
void CFitLog::vlogPeriod(int iModId, int nPriority, const char* pcFormat, va_list pArg)
{
	/*���iModId����ֵ�Ƿ�Ϸ�*/
	if ((iModId < 0) || (iModId >= m_nModId))
	{
		cout << "log: invalid modId " << iModId << endl;
		return;
	}

	/*����iModId��m_vectorModInfo���ҵ���ģ���Ӧ�ļ�¼*/
	ModInfo_t &stModInfo = m_vectorModInfo[iModId];

	/*���ȼ�̫��,����¼*/
	if (nPriority > stModInfo.nPriority)
	{
		return;
	}

	char *pcText = NULL;
	int nResult;

	/*���ɸ�ʽ������ַ���*/
	nResult = vasprintf(&pcText, pcFormat, pArg);
	//nResult = sprintf(pcText, pcFormat, pArg);

	//��ʽ���ַ�������
	if (nResult == -1)
	{
		cout << __FUNCTION__ << "call vasprintf fail" << endl;
		return;
	}

	string sText(pcText);

	/*�ͷ�vasprintf����������ڴ�*/
	free(pcText);

	//pthread_mutex_lock(&m_mutex);

	Lock();
	//���Ҹ���־��Ϣ�Ƿ��Ѿ���¼��
	std::map<string, PeriodLogInfo_t>::iterator iterator;
	iterator = m_periodLogInfo.find(sText);

	//�״γ���
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

		//�����־֮ǰ�Ѿ������������Ҫ��¼�״γ���ʱ��
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
* �������ƣ�setModPriority
* �������ܣ�����ģ����־��¼���ȼ�,ֻ�д��ڵ��ڸ����ȼ�����Ϣ�Żᱻ���
* ����˵����iModId: ģ��ţ�ģ��Ż�ȡ��μ�registerMod����
nPriority: ��־���ȼ������嶨��ֵ��ο�LOG_PRIORITY_t
* ����ֵ˵����0-��ʾִ�гɹ�; -1-��ʾִ��ʧ��
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
int CFitLog::setModPriority(int iModId, int nPriority)
{
	/*���iModId����ֵ�Ƿ�Ϸ�*/
	if ((iModId < 0) || (iModId >= m_nModId))
	{
		cout << "setModPriority: invalid modId " << iModId << endl;
		return -1;
	}

	/*����iModId��m_vectorModInfo���ҵ���ģ���Ӧ�ļ�¼*/
	ModInfo_t &stModInfo = m_vectorModInfo[iModId];

	/*���¸�ģ������ȼ�ΪnPriority*/
	stModInfo.nPriority = nPriority;

	return 0;
}

/*******************************************************************************
*
* �������ƣ�setModPriority
* �������ܣ�����ģ����־��¼���ȼ�,ֻ�д��ڵ��ڸ����ȼ�����Ϣ�Żᱻ���
* ����˵����sModName: ģ����
nPriority: ��־���ȼ������嶨��ֵ��ο�LOG_PRIORITY_t
* ����ֵ˵����0-��ʾִ�гɹ�; -1-��ʾִ��ʧ��
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
int CFitLog::setModPriority(const string sModName, int nPriority)
{
	/*��m_vectorModInfo�����pcModName�Ƿ��Ѿ�ע��*/
	int nModId;
	nModId = findModule(sModName);

	/*��ģ��û��ע��*/
	if (nModId == -1)
	{
		cout << "setModPriority: " << sModName << " has not registered." << endl;
		return -1;
	}

	/*���¸�ģ������ȼ�ΪnPriority*/
	m_vectorModInfo[nModId].nPriority = nPriority;

	return 0;
}

/*******************************************************************************
*
* �������ƣ�getModPriority
* �������ܣ���ȡģ����־��¼���ȼ�
* ����˵����sModName: ģ����
nPriority[out]: ģ�����־��¼���ȼ�
* ����ֵ˵����0-��ʾִ�гɹ�; -1-��ʾִ��ʧ��
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
int CFitLog::getModPriority(const string sModName, int& nPriority)
{
	/*��m_vectorModInfo�����pcModName�Ƿ��Ѿ�ע��*/

	int nModId;
	nModId = findModule(sModName);

	/*��ģ��û��ע��*/
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
* �������ƣ�getModPriority
* �������ܣ���ȡģ����־��¼���ȼ�
* ����˵����iModId: ģ��ţ�ģ��Ż�ȡ��μ�registerMod����
nPriority[out]: ģ�����־��¼���ȼ�
* ����ֵ˵����0-��ʾִ�гɹ�; -1-��ʾִ��ʧ��
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
int CFitLog::getModPriority(int iModId, int& nPriority)
{
	/*���iModId����ֵ�Ƿ�Ϸ�*/
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
* �������ƣ�setAllPriority
* �������ܣ���������ģ�����־��¼���ȼ�,ֻ�д��ڵ��ڸ����ȼ�����Ϣ�Żᱻ���
* ����˵����nPriority: ��־���ȼ������嶨��ֵ��ο�LOG_PRIORITY_t
* ����ֵ˵����0-��ʾִ�гɹ�; -1-��ʾִ��ʧ��
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
int CFitLog::setAllPriority(int nPriority)
{
	unsigned int i;

	/*����m_vectorModInfo*/
	for (i = 0; i < m_vectorModInfo.size(); i++)
	{
		/*����ģ������ȼ�ΪnPriority*/
		m_vectorModInfo[i].nPriority = nPriority;
	}

	return 0;
}


/*******************************************************************************
*
* �������ƣ�getLayout
* �������ܣ�����һ��PatternLayout
* ����˵������
* ����ֵ˵����Layout����ָ��
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
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
* �������ƣ�enableStdout
* �������ܣ�ʹ����־������ն�
* ����˵������
* ����ֵ˵����0-��ʾִ�гɹ�; -1-��ʾִ��ʧ��
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
int CFitLog::enableStdout(void)
{
	/*�Ѿ�ʹ����־������ն�*/
	if (m_pCoutAppender != NULL)
	{
		cout << "std output is enabled already." << endl;
		return 0;
	}

	/*����log4cpp::OstreamAppender����*/
	m_pCoutAppender = new log4cpp::OstreamAppender("OstreamAppender", &cout);
	if (m_pCoutAppender == NULL)
	{
		cout << __FUNCTION__ << "alloc mem fail" << endl;
		return -1;
	}

	/*����һ��layout*/
	log4cpp::Layout* pLayout = getLayout();

	/*����m_pCoutAppender��Layout*/
	m_pCoutAppender->setLayout(pLayout);

	/*��m_pCoutAppender��ӵ�m_pRootCategory��*/
	m_pRootCategory->addAppender(m_pCoutAppender);

	cout << "enable std output success" << endl;

	return 0;
}

/*******************************************************************************
*
* �������ƣ�disableStdOutput
* �������ܣ���ֹ��־������ն�
* ����˵������
* ����ֵ˵����0-��ʾִ�гɹ�; -1-��ʾִ��ʧ��
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
int CFitLog::disableStdOutput(void)
{
	/*��û��ʹ����־������ն�*/
	if (m_pCoutAppender == NULL)
	{
		cout << "std output is disabled." << endl;
		return 0;
	}

	/*��m_pCoutAppender��m_pRootCategory��ɾ��*/
	m_pRootCategory->removeAppender(m_pCoutAppender);

	delete m_pCoutAppender;
	m_pCoutAppender = NULL;

	cout << "disable std output success" << endl;

	return 0;
}

/*******************************************************************************
*
* �������ƣ�enableMemOutput
* �������ܣ�ʹ����־������ڴ�
* ����˵������
* ����ֵ˵����0-��ʾִ�гɹ�; -1-��ʾִ��ʧ��
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
int CFitLog::enableMemOutput(void)
{
	/*�Ѿ�ʹ����־������ڴ�*/
	if (m_pMemAppender != NULL)
	{
		cout << "memory output is enabled already." << endl;
		return 0;
	}

	/*����log4cpp::StringQueueAppender����*/
	m_pMemAppender = new log4cpp::StringQueueAppender("StringQueueAppender");
	if (m_pMemAppender == NULL)
	{
		cout << __FUNCTION__ << "alloc mem fail" << endl;
		return -1;
	}

	/*����һ��layout*/
	log4cpp::Layout* pLayout = getLayout();

	/*����m_pMemAppender��Layout*/
	m_pMemAppender->setLayout(pLayout);

	/*��m_pMemAppender��ӵ�m_pRootCategory��*/
	m_pRootCategory->addAppender(m_pMemAppender);

	cout << "enable memory output success" << endl;

	return 0;
}

/*******************************************************************************
*
* �������ƣ�disableMemOutput
* �������ܣ���ֹ��־������ڴ�
* ����˵������
* ����ֵ˵����0-��ʾִ�гɹ�; -1-��ʾִ��ʧ��
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
int CFitLog::disableMemOutput(void)
{
	/*��û��ʹ����־������ڴ�*/
	if (m_pMemAppender == NULL)
	{
		cout << "mem output is disabled already." << endl;
		return 0;
	}

	/*��m_pMemAppender��m_pRootCategory��ɾ��*/
	m_pRootCategory->removeAppender(m_pMemAppender);

	delete m_pMemAppender;
	m_pMemAppender = NULL;

	cout << "disable memory output success" << endl;

	return 0;
}

/*******************************************************************************
*
* �������ƣ�getMemLog
* �������ܣ���ȡ�������ڴ��е���־��Ϣ
* ����˵����strQuque[out]: ��ȡ������־��Ϣ�б�
* ����ֵ˵����0-��ʾִ�гɹ�; -1-��ʾִ��ʧ��
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
int CFitLog::getMemLog(queue<string>& strQuque)
{
	/*��û��ʹ����־������ڴ�*/
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
* �������ƣ�enableFileOutput
* �������ܣ�ʹ����־������ļ�
* ����˵����pcFileName: ��־�ļ���
* ����ֵ˵����0-��ʾִ�гɹ�; -1-��ʾִ��ʧ��
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
int CFitLog::enableFileOutput(string strFileName)
{
	/*�Ѿ�ʹ����־������ļ�*/
	if (m_pFileAppender != NULL)
	{
		cout << "file output is enabled already." << endl;
		return 0;
	}

	/*����log4cpp::FileAppender����*/
	m_pFileAppender = new log4cpp::FileAppender("FileAppender", strFileName);
	if (m_pFileAppender == NULL)
	{
		cout << __FUNCTION__ << "alloc mem fail" << endl;
		return -1;
	}

	/*����һ��layout*/
	log4cpp::Layout* pLayout = getLayout();

	/*����m_pFileAppender��Layout*/
	m_pFileAppender->setLayout(pLayout);

	/*��m_pFileAppender��ӵ�m_pRootCategory��*/
	m_pRootCategory->addAppender(m_pFileAppender);

	cout << "enable file output success" << endl;

	return 0;
}

/*******************************************************************************
*
* �������ƣ�disableFileOutput
* �������ܣ���ֹ��־������ļ�
* ����˵����
* ����ֵ˵����0-��ʾִ�гɹ�; -1-��ʾִ��ʧ��
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
int CFitLog::disableFileOutput(void)
{
	/*��û��ʹ����־������ļ�*/
	if (m_pFileAppender == NULL)
	{
		cout << "file output is disabled already." << endl;
		return 0;
	}

	/*��m_pFileAppender��m_pRootCategory��ɾ��*/
	m_pRootCategory->removeAppender(m_pFileAppender);

	delete m_pFileAppender;
	m_pFileAppender = NULL;

	cout << "disable file output success" << endl;
	return 0;
}

/*******************************************************************************
*
* �������ƣ�enableRollingFileOutput
* �������ܣ�ʹ����־������ؾ��ļ�
* ����˵����pcFileName: ��־�ļ���
maxFileSize: ��־�ļ����ֵ, ��λ: Byte
uMaxBackupIndex: ��־�ļ���󱸷ݸ���
* ����ֵ˵����0-��ʾִ�гɹ�; -1-��ʾִ��ʧ��
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
int CFitLog::enableRollingFileOutput(string strFileName,
																		 size_t maxFileSize, unsigned int uMaxBackupIndex)
{
	/*�Ѿ�ʹ����־������ؾ��ļ�*/
	if (m_pRollingFileAppender != NULL)
	{
		cout << "rolling file output is enabled already." << endl;
		return 0;
	}
	
	/*����log4cpp::RollingFileAppender����*/
	m_pRollingFileAppender = new log4cpp::RollingFileAppender("RollingFileAppender",strFileName, maxFileSize, uMaxBackupIndex);
	if (m_pRollingFileAppender == NULL)
	{
		cout << __FUNCTION__ << "alloc mem fail" << endl;
		return -1;
	}

	/*����һ��layout*/
	log4cpp::Layout* pLayout = getLayout();

	/*����m_pRollingFileAppender��Layout*/
	m_pRollingFileAppender->setLayout(pLayout);

	/*��m_pRollingFileAppender��ӵ�m_pRootCategory��*/
	m_pRootCategory->addAppender(m_pRollingFileAppender);

	cout << "enable rolling file output success" << endl;
	return 0;
}

/*******************************************************************************
*
* �������ƣ�disableRollingFileOutput
* �������ܣ���ֹ��־������ؾ��ļ�
* ����˵����
* ����ֵ˵����0-��ʾִ�гɹ�; -1-��ʾִ��ʧ��
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
int CFitLog::disableRollingFileOutput(void)
{
	/*��û��ʹ����־������ؾ��ļ�*/
	if (m_pRollingFileAppender == NULL)
	{
		cout << "rolling file output is disabled already." << endl;
		return 0;
	}

	/*��m_pRollingFileAppender��m_pRootCategory��ɾ��*/
	m_pRootCategory->removeAppender(m_pRollingFileAppender);

	delete m_pRollingFileAppender;
	m_pRollingFileAppender = NULL;

	cout << "disable rolling file output success" << endl;
	return 0;
}

/*******************************************************************************
*
* �������ƣ�setPeriodLogFlushInterval
* �������ܣ�������������־ǿ���������־�ļ��е�ʱ����
* ����˵����nSeconds: ʱ����, ��λ: ��
* ����ֵ˵����None
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
void CFitLog::setPeriodLogFlushInterval(int nSeconds)
{
	m_nFlushInterval = nSeconds;
}

/*******************************************************************************
*
* �������ƣ�flushPeriodLog
* �������ܣ����������ڴ��е���������־�������־�ļ���
* ����˵����
* ����ֵ˵����None
* �޸��б���YYYY-MM-DD hh:mm by xxxx)
* �����˵����
* ������
*
******************************************************************************/
void CFitLog::flushPeriodLog()
{
	time_t lTime = time(NULL);

	//ˢ��ʱ��δ��
	if (lTime - m_lastChkPeriodLogTime < FLUSH_CHECK_INTERVAL)
	{
		return;
	}
	m_lastChkPeriodLogTime = lTime;

	std::map<string, PeriodLogInfo_t>::iterator iterator;

	//�������е���������־��Ϣ
	for (iterator = m_periodLogInfo.begin(); iterator != m_periodLogInfo.end(); iterator++)
	{
		PeriodLogInfo_t &stLogInfo = iterator->second;

		//��־����ǿ�����ʱ����
		if ((stLogInfo.lCount > 0) && (lTime - stLogInfo.lTime > m_nFlushInterval))
		{
			//�����־��Ϣ
			string sText(iterator->first);

			//��־������ظ�����ֵ
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
/* ����ν��� */

