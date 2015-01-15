#include <windows.h>
#include <iostream>
using namespace std;

#define GUIDECONSOLEWATCH  "GuideConsoleWatch"
#define GUIDECONSOLEWATCH_SERVICE  "GuideConsoleWatch_Service"

void ErrorHandler(char *s, DWORD err)
{
    cout<< s << endl;
    cout<< "Error number: " << err << endl;
    ExitProcess(err);
}

int main(int argc, char *argv[])
{
    SC_HANDLE newService, scm;
    
    // open a connection to the SCM
    scm = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);
    if (!scm) ErrorHandler("In OpenScManager",
        GetLastError());
    
    // Install the new service
    newService = CreateService(
        scm, GUIDECONSOLEWATCH, // eg "beep_srv"
        GUIDECONSOLEWATCH_SERVICE, // eg "Beep Service"
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
        SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
        "C:\\pgTest\\service\\memCheck\\Debug\\memCheck.exe", // eg "c:\winnt\xxx.exe"
        0, 0, 0, 0, 0);
    if (!newService) ErrorHandler("In CreateService",
        GetLastError());
    else cout << "Service installed\n";
    
    // clean up
    CloseServiceHandle(newService);
    CloseServiceHandle(scm);

	return 0;
}