
#include <iostream>
#include <fstream>
#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <stdarg.h>
#pragma comment(lib,"Psapi.lib")
using namespace std;

#define LOCAL_LOG_MAX_SIZE 1024
#define GUIDECONSOLEWATCH  "GuideConsoleWatch"
#define TIME_BUFF			128

SERVICE_STATUS          gSvcStatus;			//サービス 
SERVICE_STATUS_HANDLE   gSvcStatusHandle;	//サービスハンドル 

void WriteLogToFile(LPSTR format, ...);
void WINAPI GuideConsoleWatchMain(DWORD dwArgc, LPTSTR *lpszArgv);
VOID WINAPI GuideConsoleWatchCtrlHandler(DWORD);

int showMemoryInfo(const char* processName)
{
	ofstream fout("C:\\pgTest\\service\\mem_log.txt", ios::app);
	DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return 0;

    // Calculate how many process identifiers were returned.

    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the memory usage for each process
	HANDLE hProcess;
	HMODULE hMod;
	DWORD arraySize;
	char m_processName[512];
		
    for ( i = 0; i < cProcesses; i++ )
	{
		DWORD m_processid = aProcesses[i];
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, m_processid);
		if(hProcess)
		{
		    if (EnumProcessModules(hProcess,&hMod,sizeof(HMODULE),&arraySize))
			{
				memset(m_processName, 0x00, sizeof(m_processName));
				GetModuleBaseName(hProcess, hMod, m_processName, sizeof(m_processName));
			}
			if (0 == strncmp(processName, m_processName, sizeof(m_processName)))
			{
				break;
			}
		}
	}
	//HANDLE handle=GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));

	SYSTEMTIME now;
	GetSystemTime(&now);
	char time[TIME_BUFF];
	memset(time, 0x00, TIME_BUFF);
	sprintf(time, "%04d-%02d-%02d %02d:%02d:%02d", now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond);
    fout<<"memory"<<"["<<time<<"]:"<<endl;
	fout<<"Working set:"<<pmc.WorkingSetSize/1000 <<"K/"<<pmc.PeakWorkingSetSize/1000<<"K + PageFile:"<<pmc.PagefileUsage/1000 <<"K/"<<pmc.PeakPagefileUsage/1000 <<"K"<<endl;
	
	fout.close();
	return 1;
}

int main(int argc,char* argv)
{
	WriteLogToFile("%s", "This is main!");

	SERVICE_TABLE_ENTRY DispatchTable[] =
	{
		{ GUIDECONSOLEWATCH, (LPSERVICE_MAIN_FUNCTION)GuideConsoleWatchMain },
		{ NULL, NULL }
	};

	if (!StartServiceCtrlDispatcher(DispatchTable))
	{
		WriteLogToFile("サービスディスパッチが失敗した！");
	}
	showMemoryInfo("GuideConsole.exe");
    return 0;
}


void WriteLogToFile(LPSTR format, ...)
{
	va_list list;

	va_start(list, format);

	char logMsg_buf[LOCAL_LOG_MAX_SIZE];
	char logMsg[LOCAL_LOG_MAX_SIZE];

	_vsnprintf(logMsg_buf, LOCAL_LOG_MAX_SIZE, format, list);

	ofstream fout_l("C:\\pgTest\\service\\out_log.txt", ios::app);
	fout_l.clear();
	SYSTEMTIME now;
	GetSystemTime(&now);

//	fout_l.seekp(0, ios::end);
	sprintf(logMsg, "%04d-%02d-%02d %02d:%02d:%02d %s", now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond, logMsg_buf);

	fout_l<<logMsg<<endl;
	fout_l.close();
}


void WINAPI GuideConsoleWatchMain(DWORD dwArgc, LPTSTR *lpszArgv )
{
	if (!(gSvcStatusHandle = RegisterServiceCtrlHandler(GUIDECONSOLEWATCH,
                     GuideConsoleWatchCtrlHandler)))
	return;
}


VOID WINAPI GuideConsoleWatchCtrlHandler(DWORD signal)
{
}